#include <Arduino.h>
#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int homing_M2_Speed = 238;
int homing_M1_Speed = 250;

// Maximum and minimum speed limits for motors
int MIN_SPEED = 85; // tuned
int MAX_SPEED = 150; // tuned

float Ksync = 0.5f; // tune

float KsyncI = 0.05f;

float syncIntegral = 0.0f;

// Minimum real-world duration (ms) a sub-MIN_SPEED duty pulse must stay ON
// to reliably overcome motor stiction and produce actual movement. Too short
// and the motor never actually turns (permanent stall); too long and the
// crawl becomes coarse/jerky. Bench-tune this per motor/load.
unsigned long minPulseMs = 60; // starting guess — tune on hardware

// PI Variables
// Controller gains (kp + ki > 0.05 for error = 100mm [~3000 encoder counts] to output min. speed of 75)
float Kp = 0.10f; // tune
float Ki = 0.002f; // tune
int positionTolerance = 200; //adjust based on testing

// Velocity profile acceleration constant (tune on the bench alongside Kp/Ki)
float accelConstant = 1.0f; // placeholder — tune

MotionController::MotionController(Encoder& encoder, float& absoluteX, float& absoluteY, volatile SwitchState& last_pressed) : encoder(encoder), absoluteX(absoluteX), absoluteY(absoluteY), last_pressed(last_pressed) {
    targetX = 0.0f;
    targetY = 0.0f;
    speed = 0.0f;

    targetMotor1 = 0;
    targetMotor2 = 0;

    prevIntegralMotor1 = 0;
    prevIntegralMotor2 = 0;

    integralMotor1 = 0;
    integralMotor2 = 0;

    moving_completed = true;
}

void MotionController::setTarget(float x, float y, float speed) {
    targetX = x;
    targetY = y;
    this->speed = speed;

    calculateMotorTargets();
    
    moving_completed = false;
}

void MotionController::update() { // This controls the motors

    if (moving_completed) {
        Serial.println("Motion completed");
        return;
    }

    // Update current motor positions (these are relative to the position when motion was called; starts at 0 and increases to target)
    long currentMotor1 = encoder.getMotor1Count();
    long currentMotor2 = encoder.getMotor2Count();

    // Calculate errors (direction of motion is determined by the sign of the error)
    long motor1Error = targetMotor1 - currentMotor1;
    long motor2Error = targetMotor2 - currentMotor2;

    // --- Velocity profile: compute the synchronized ceiling for this tick ---
    float dx = (float)((currentMotor1 - startMotor1) + (currentMotor2 - startMotor2)) / 2.0f;
    float dy = (float)((currentMotor2 - startMotor2) - (currentMotor1 - startMotor1)) / 2.0f;
    float dPath = sqrt(dx * dx + dy * dy);

    float remainingPath = pathLengthCounts - dPath;
    if (remainingPath < 0.0f) remainingPath = 0.0f;

    float vAccel = calculateVelocityCeiling(dPath);        // ramps up from MIN_SPEED
    float vDecel = calculateVelocityCeiling(remainingPath); // ramps down toward MIN_SPEED near target
    float vPath = min(vAccel, vDecel);

    float ceiling1 = speed;
    float ceiling2 = speed;
    if (pathLengthCounts > 0.0f) {
        ceiling1 = vPath * (dist1Total / pathLengthCounts);
        ceiling2 = vPath * (dist2Total / pathLengthCounts);

        float worst = max(ceiling1, ceiling2);
        if (worst > MAX_SPEED) {
            float scale = MAX_SPEED / worst;
            ceiling1 *= scale;
            ceiling2 *= scale;
        }
    }

    // --- Real-time sync correction ---
    // progress1/progress2: fraction (0..1) of each motor's OWN total distance
    // covered so far. Comparing fractions (not raw counts) is what makes this
    // work correctly even when dist1Total != dist2Total, e.g. on diagonals.
    float progress1 = (dist1Total > 0.0f) ? (float)abs(currentMotor1 - startMotor1) / dist1Total : 1.0f;
    float progress2 = (dist2Total > 0.0f) ? (float)abs(currentMotor2 - startMotor2) / dist2Total : 1.0f;

    float syncError = progress1 - progress2;

    syncIntegral += syncError;
    // Anti-windup: keep the integral term from growing unbounded
    if (syncIntegral > 1.5f) syncIntegral = 1.5f;
    if (syncIntegral < -1.5f) syncIntegral = -1.5f;

    float syncCorrection = Ksync * syncError + KsyncI * syncIntegral;

    ceiling1 *= (1.0f - syncCorrection);
    ceiling2 *= (1.0f + syncCorrection);

    if (ceiling1 < 0.0f) ceiling1 = 0.0f;
    if (ceiling2 < 0.0f) ceiling2 = 0.0f;

    // Re-clamp jointly so the sync correction can't push one motor over
    // MAX_SPEED while leaving the other alone — that broke the ratio
    // between them exactly in the cruise phase (mid-move), causing the
    // visible gradient/direction change on long diagonal moves.
    float worstPostSync = max(ceiling1, ceiling2);
    if (worstPostSync > MAX_SPEED) {
        float postSyncScale = MAX_SPEED / worstPostSync;
        ceiling1 *= postSyncScale;
        ceiling2 *= postSyncScale;
    }

    // MOTOR 1 CONTROL //
    if (abs(motor1Error) <= positionTolerance)  {
        // Stop motor 1
        digitalWrite(motor1_pin, 0);
        analogWrite(enable1_pin, 0);

        // Reset integral terms
        integralMotor1 = 0;
        prevIntegralMotor1 = 0;
        dutyAccumulator1 = 0.0f;
        pulseActive1 = false;
    } else {
        // Integral terms for PID control
        integralMotor1 = prevIntegralMotor1 + Ki * motor1Error;
        // Motor output
        float speedMotor1 = abs(Kp * motor1Error + integralMotor1); // must be positive for analogWrite

        // Integral clamping to prevent windup
        if (speedMotor1 > ceiling1) {
            speedMotor1 = ceiling1;
            integralMotor1 = prevIntegralMotor1; // Clamp: DON'T update integral!
        } else {
            prevIntegralMotor1 = integralMotor1; // Only update previous integral if not saturated
        }

        if (speedMotor1 > MAX_SPEED) speedMotor1 = MAX_SPEED;

        if (speedMotor1 < MIN_SPEED) {
            // Sub-deadband crawl: continuous PWM this low won't reliably turn
            // the motor. Instead, fire full MIN_SPEED pulses and HOLD each
            // pulse on for at least minPulseMs of real time (not just one
            // update() tick) so the motor actually has time to overcome
            // static friction and move, rather than twitching and stalling.
            if (pulseActive1) {
                speedMotor1 = MIN_SPEED;
                if (millis() - pulseStartMillis1 >= minPulseMs) {
                    pulseActive1 = false;
                }
            } else {
                dutyAccumulator1 += speedMotor1 / MIN_SPEED;
                if (dutyAccumulator1 >= 1.0f) {
                    dutyAccumulator1 -= 1.0f;
                    pulseActive1 = true;
                    pulseStartMillis1 = millis();
                    speedMotor1 = MIN_SPEED;
                } else {
                    speedMotor1 = 0.0f;
                }
            }
        } else {
            dutyAccumulator1 = 0.0f; // not in crawl mode — don't carry stale credit
            pulseActive1 = false;
        }

        // MOTOR 1 CONTROL //
        digitalWrite(motor1_pin, motor1Error > 0 ? HIGH : LOW);
        analogWrite(enable1_pin, speedMotor1);
    }

    // MOTOR 2 CONTROL //
    if (abs(motor2Error) <= positionTolerance) {
        // Stop motor 2
        digitalWrite(motor2_pin, 0);
        analogWrite(enable2_pin, 0);

        // Reset integral terms
        integralMotor2 = 0;
        prevIntegralMotor2 = 0;
        dutyAccumulator2 = 0.0f;
        pulseActive2 = false;
    } else {
        // Integral terms for PID control
        integralMotor2 = prevIntegralMotor2 + Ki * motor2Error;

        // Calculate motor speed outputs
        float speedMotor2 = abs(Kp * motor2Error + integralMotor2);

        // Integral clamping to prevent windup
        if (speedMotor2 > ceiling2) {
            speedMotor2 = ceiling2;
            integralMotor2 = prevIntegralMotor2; // Clamp: DON'T update integral!
        } else {
            prevIntegralMotor2 = integralMotor2; // Only update previous integral if not saturated
        }

        if (speedMotor2 > MAX_SPEED) speedMotor2 = MAX_SPEED;

        if (speedMotor2 < MIN_SPEED) {
            // Sub-deadband crawl — see motor1 comment above.
            if (pulseActive2) {
                speedMotor2 = MIN_SPEED;
                if (millis() - pulseStartMillis2 >= minPulseMs) {
                    pulseActive2 = false;
                }
            } else {
                dutyAccumulator2 += speedMotor2 / MIN_SPEED;
                if (dutyAccumulator2 >= 1.0f) {
                    dutyAccumulator2 -= 1.0f;
                    pulseActive2 = true;
                    pulseStartMillis2 = millis();
                    speedMotor2 = MIN_SPEED;
                } else {
                    speedMotor2 = 0.0f;
                }
            }
        } else {
            dutyAccumulator2 = 0.0f;
            pulseActive2 = false;
        }

        // MOTOR 2 CONTROL //
        digitalWrite(motor2_pin, motor2Error > 0 ? HIGH : LOW);
        analogWrite(enable2_pin, speedMotor2);
    }

    if (abs(motor1Error) <= positionTolerance && abs(motor2Error) <= positionTolerance) {
        updateAbsolutePosition();

        if(!homingRunning) {
            Serial.println("=== Movement Complete ===");
            Serial.print("Absolute X: ");
            Serial.println(absoluteX);
            Serial.print("Absolute Y: ");
            Serial.println(absoluteY);

        }

        moving_completed = true;
        
    }
}


bool MotionController::isCompleted() const {
    return moving_completed;
}

void MotionController::calculateMotorTargets() {
    long motor1Counts = encoder.convertToCounts(targetX - targetY);
    long motor2Counts = encoder.convertToCounts(targetX + targetY);


    // Snapshot starting position for this move (velocity profile reference point)
    startMotor1 = encoder.getMotor1Count();
    startMotor2 = encoder.getMotor2Count();

    // Convert target positions to encoder counts
    targetMotor1 = startMotor1 + motor1Counts;
    targetMotor2 = startMotor2 + motor2Counts;

    // Velocity profile: per-motor distance and true Cartesian path length for this move
    dist1Total = abs((float)motor1Counts);
    dist2Total = abs((float)motor2Counts);
    pathLengthCounts = encoder.convertToCounts(sqrt(targetX * targetX + targetY * targetY));

    syncIntegral = 0.0f;
    dutyAccumulator1 = 0.0f;
    dutyAccumulator2 = 0.0f;
    pulseActive1 = false;
    pulseActive2 = false;

}

void MotionController::updateAbsolutePosition() {
    absoluteX += targetX;
    absoluteY += targetY;

}

float MotionController::calculateVelocityCeiling(float distanceTraveled) const {
    // Accel phase: ramps up from MIN_SPEED as distance traveled increases
    float accelPhase = sqrt(MIN_SPEED * MIN_SPEED + 2.0f * accelConstant * distanceTraveled);


    // Decel phase: ramps down toward MIN_SPEED as distance remaining shrinks
    float remaining = pathLengthCounts - distanceTraveled;
    if (remaining < 0.0f) {
        remaining = 0.0f;
    }
    float decelPhase = sqrt(MIN_SPEED * MIN_SPEED + 2.0f * accelConstant * remaining);


    // Whichever phase is more restrictive at this point in the move wins
    float v = accelPhase;
    if (decelPhase < v) {
        v = decelPhase;
    }
    if (v > speed) {
        v = speed;
    }
    return v;

}

bool MotionController::debugTick() {
    unsigned long now = millis();
    if (now - lastDebugPrint >= DEBUG_PRINT_INTERVAL_MS) {
        lastDebugPrint = now;
        return true;
    }
    return false;
}

// State functions
void MotionController::Idle() {
    // Stop both motors
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void MotionController::HomingIdle(){
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void MotionController::HomingFunction() {

    // Homing state machine
    homingRunning = true;
    switch (homingState) {
        case MOVE_TO_LEFT:
            if (last_pressed == sB) {
                last_pressed = START;
                setTarget(0.0f, 5.0f, 75.0f);
                homingState = BOTTOM_EDGE_CASE_WAIT;
            }else if (last_pressed == sL) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_RIGHT;
            } else {
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, homing_M1_Speed);
                analogWrite(enable2_pin, homing_M2_Speed);
            }
            break;


        case BOTTOM_EDGE_CASE_WAIT:
            update();
            if (isCompleted()) {
                homingState = MOVE_TO_LEFT;
            }
            break;


        case MOVE_RIGHT:
            setTarget(10.0f, 0.0f, 250.0f);
            homingState = WAIT_AFTER_RIGHT;
            break;


        case WAIT_AFTER_RIGHT:
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_LEFT2;
            }
            break;


        case MOVE_TO_LEFT2:
            if (last_pressed == sL) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_RIGHT2;
            } else {
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, 98);
                analogWrite(enable2_pin, 75);

            }
            break;
       
        case MOVE_RIGHT2:
            setTarget(5.0f, 0.0f, 98.0f);
            homingState = WAIT_AFTER_RIGHT2;
            break;


        case WAIT_AFTER_RIGHT2:
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_BOTTOM;
            }
            break;


        case MOVE_TO_BOTTOM:
            if (last_pressed == sB) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_UP;
            } else {
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, homing_M1_Speed);
                analogWrite(enable2_pin, homing_M2_Speed);
            }
            break;


        case MOVE_UP:
            setTarget(0.0f, 10.0f, 250.0f);
            homingState = WAIT_AFTER_UP;
            break;
       
        case WAIT_AFTER_UP:
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_BOTTOM2;
            }
            break;
       
        case MOVE_TO_BOTTOM2:
            if (last_pressed == sB) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_UP2;
            } else {
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, 98);
                analogWrite(enable2_pin, 75);
            }
            break;

        case MOVE_UP2:
            setTarget(0.0f, 5.0f, 98.0f);
            homingState = WAIT_AFTER_UP2;
            break;
       
        case WAIT_AFTER_UP2:
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = HOMING_COMPLETE;
            }
            break;

        case HOMING_COMPLETE:
            update();

            if (isCompleted()) {
                Serial.println("=== HOMING COMPLETE ===");
                HomingIdle();
                absoluteX = 0.0f;
                absoluteY = 0.0f;
                encoder.resetCounts();
                homingComplete = true;
                homingRunning = false;
            }
            break;
    }
}


bool MotionController::isHomingComplete() const {
    return homingComplete;
}

void MotionController::StartHoming() {
    homingState = MOVE_TO_LEFT;
    homingComplete = false;
    moving_completed = true;

    integralMotor1 = 0;
    integralMotor2 = 0;
    prevIntegralMotor1 = 0;
    prevIntegralMotor2 = 0;
}