#include <Arduino.h>
#include "motionController.h"

// Motor control pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing motor speeds
int homing_M2_Speed = 238;
int homing_M1_Speed = 250;

// Motor speed limits (PWM range: 0-255)
int MIN_SPEED = 85;  
int MAX_SPEED = 150;   

// Synchronisation control gains to allow for synchronised movement
float Ksync = 0.5f; // proportional gain for sync error
float KsyncI = 0.05f; // integral gain for sync error
float syncIntegral = 0.0f; // integral accumulator for sync correction

// Low-speed pulse mode timing
unsigned long minPulseMs = 60; // tuned so motor can spin but not cause jerk

// PI control parameters (proportional and integral gains)
// Must satisfy: (Kp + Ki) > 0.05 for 100mm error (~3000 counts) to output MIN_SPEED
float Kp = 0.10f;  // proportional gain
float Ki = 0.002f; // integral gain 
int positionTolerance = 200;  // acceptable position error (encoder counts)

// Acceleration profile constant used in velocity ceiling calculation: 
// v = sqrt(MIN_SPEED^2 + 2 * accel * distance)
float accelConstant = 1.0f; 

// Constructor: Initialise motion controller with system references
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

// Set target position and speed for next movement
void MotionController::setTarget(float x, float y, float speed) {
    targetX = x;
    targetY = y;
    this->speed = speed;

    // Calculate required motor movements from XY target
    calculateMotorTargets();
    
    moving_completed = false;
}

// Main control loop: execute one iteration of motor control it controls the motors/motion

void MotionController::update() { 

    if (moving_completed) {
        Serial.println("Motion completed");
        return;
    }

    // Get current encoder positions
    long currentMotor1 = encoder.getMotor1Count();
    long currentMotor2 = encoder.getMotor2Count();

    // Calculate position errors (motor error indicates direction of needed movement)
    long motor1Error = targetMotor1 - currentMotor1;
    long motor2Error = targetMotor2 - currentMotor2;

    // Velocity Profile Calculation
    // Synchronize motors along cartesian path: ramp up acceleration, ramp down for deceleration

    // dx, dy represent decomposed progress along motor axes
    float dx = (float)((currentMotor1 - startMotor1) + (currentMotor2 - startMotor2)) / 2.0f;
    float dy = (float)((currentMotor2 - startMotor2) - (currentMotor1 - startMotor1)) / 2.0f;
    float dPath = sqrt(dx * dx + dy * dy);  // Distance traveled along path

    float remainingPath = pathLengthCounts - dPath;
    if (remainingPath < 0.0f) remainingPath = 0.0f;

    // Acceleration and deceleration curves
    float vAccel = calculateVelocityCeiling(dPath); // Ramps up from MIN_SPEED
    float vDecel = calculateVelocityCeiling(remainingPath); // Ramps down toward MIN_SPEED
    float vPath = min(vAccel, vDecel); // Use most restrictive limit

    // Convert path velocity to per motor velocity based on relative distances
    float ceiling1 = speed;
    float ceiling2 = speed;
    if (pathLengthCounts > 0.0f) {
        ceiling1 = vPath * (dist1Total / pathLengthCounts);
        ceiling2 = vPath * (dist2Total / pathLengthCounts);

        // Re clamp to MAX_SPEED if needed
        float worst = max(ceiling1, ceiling2);
        if (worst > MAX_SPEED) {
            float scale = MAX_SPEED / worst;
            ceiling1 *= scale;
            ceiling2 *= scale;
        }
    }

    // Synchronized Motion Correction
    // Track progress as fraction of each motor's total distance
    float progress1 = (dist1Total > 0.0f) ? (float)abs(currentMotor1 - startMotor1) / dist1Total : 1.0f;
    float progress2 = (dist2Total > 0.0f) ? (float)abs(currentMotor2 - startMotor2) / dist2Total : 1.0f;

    float syncError = progress1 - progress2;  // Which motor is ahead?

    // Integral control for sync (with anti-windup)
    syncIntegral += syncError;
    if (syncIntegral > 1.5f) syncIntegral = 1.5f;
    if (syncIntegral < -1.5f) syncIntegral = -1.5f;

    // Apply sync correction (slow fast motor, speed up slow motor)
    float syncCorrection = Ksync * syncError + KsyncI * syncIntegral;

    ceiling1 *= (1.0f - syncCorrection);
    ceiling2 *= (1.0f + syncCorrection);

    if (ceiling1 < 0.0f) ceiling1 = 0.0f;
    if (ceiling2 < 0.0f) ceiling2 = 0.0f;

    // Re clamp motors to maintain ratio and not exceed MAX_SPEED
    float worstPostSync = max(ceiling1, ceiling2);
    if (worstPostSync > MAX_SPEED) {
        float postSyncScale = MAX_SPEED / worstPostSync;
        ceiling1 *= postSyncScale;
        ceiling2 *= postSyncScale;
    }

    // Motor 1 PI Control 
    if (abs(motor1Error) <= positionTolerance)  {
        // Target reached: stop motor and reset 
        digitalWrite(motor1_pin, 0);
        analogWrite(enable1_pin, 0);

        // Reset integral terms
        integralMotor1 = 0;
        prevIntegralMotor1 = 0;
        dutyAccumulator1 = 0.0f;
        pulseActive1 = false;
    } else {
        // PI control output calculation
        integralMotor1 = prevIntegralMotor1 + Ki * motor1Error;
        // Speed output must be positive for analogWrite
        float speedMotor1 = abs(Kp * motor1Error + integralMotor1);

        // Integral anti-windup: don't update integral if output is saturated
        if (speedMotor1 > ceiling1) {
            speedMotor1 = ceiling1;
            integralMotor1 = prevIntegralMotor1; // Clamp: don't integrate further
        } else {
            prevIntegralMotor1 = integralMotor1; // Update integral only if not saturated
        }

        if (speedMotor1 > MAX_SPEED) speedMotor1 = MAX_SPEED;

        if (speedMotor1 < MIN_SPEED) {
            // Low-speed pulse mode, this gives the motor time to overcome static friction and actually move
            if (pulseActive1) {
                speedMotor1 = MIN_SPEED;
                if (millis() - pulseStartMillis1 >= minPulseMs) {
                    pulseActive1 = false;  // Pulse duration complete, stop motor
                }
            } else {
                // Accumulate fractional duty cycle
                dutyAccumulator1 += speedMotor1 / MIN_SPEED;
                if (dutyAccumulator1 >= 1.0f) {
                    dutyAccumulator1 -= 1.0f;
                    pulseActive1 = true;
                    pulseStartMillis1 = millis();
                    speedMotor1 = MIN_SPEED;
                } else {
                    speedMotor1 = 0.0f;  // Not enough duty accumulated yet
                }
            }
        } else {
            dutyAccumulator1 = 0.0f; // Not in pulse mode, reset accumulator
            pulseActive1 = false;
        }

        // Set motor direction and speed via PWM
        digitalWrite(motor1_pin, motor1Error > 0 ? HIGH : LOW);
        analogWrite(enable1_pin, speedMotor1);
    }

    // Motor 2 PI Control , similar to Motor 1
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
        // PI control for motor 2
        integralMotor2 = prevIntegralMotor2 + Ki * motor2Error;

        // Calculate motor speed outputs
        float speedMotor2 = abs(Kp * motor2Error + integralMotor2);

        // Integral clamping to prevent windup
        if (speedMotor2 > ceiling2) {
            speedMotor2 = ceiling2;
            integralMotor2 = prevIntegralMotor2;
        } else {
            prevIntegralMotor2 = integralMotor2;
        }

        if (speedMotor2 > MAX_SPEED) speedMotor2 = MAX_SPEED;

        if (speedMotor2 < MIN_SPEED) {
            // Sub-deadband pulse mode
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

        // Set motor direction and speed
        digitalWrite(motor2_pin, motor2Error > 0 ? HIGH : LOW);
        analogWrite(enable2_pin, speedMotor2);
    }

    // Check if both motors have reached target
    if (abs(motor1Error) <= positionTolerance && abs(motor2Error) <= positionTolerance) {
        // Update absolute position and mark movement as complete
        updateAbsolutePosition();

        if(!homingRunning) {
            // Print movement completion status
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

// Calculate target encoder counts for each motor from XY displacement

// Motor 1 drives movement in direction 1 (difference of X and Y)
// Motor 2 drives movement in direction 2 (sum of X and Y)
void MotionController::calculateMotorTargets() {
    // Convert XY target to motor counts
    long motor1Counts = encoder.convertToCounts(targetX - targetY);
    long motor2Counts = encoder.convertToCounts(targetX + targetY);

    
    startMotor1 = encoder.getMotor1Count();
    startMotor2 = encoder.getMotor2Count();

    // Calculate absolute target positions
    targetMotor1 = startMotor1 + motor1Counts;
    targetMotor2 = startMotor2 + motor2Counts;

    // Per motor distances and total Cartesian path length
    dist1Total = abs((float)motor1Counts);
    dist2Total = abs((float)motor2Counts);
    pathLengthCounts = encoder.convertToCounts(sqrt(targetX * targetX + targetY * targetY));

    // Reset sync and pulse mode accumulators for this new movement
    syncIntegral = 0.0f;
    dutyAccumulator1 = 0.0f;
    dutyAccumulator2 = 0.0f;
    pulseActive1 = false;
    pulseActive2 = false;

}

// Update absolute position after movement completes
void MotionController::updateAbsolutePosition() {
    // Add completed movement displacement to absolute position
    absoluteX += targetX;
    absoluteY += targetY;

}

// Calculate maximum allowed speed at given distance along path
// Implements trapezoidal velocity profile: acceleration phase, cruise, deceleration
float MotionController::calculateVelocityCeiling(float distanceTraveled) const {
    // Acceleration phase: ramps up from MIN_SPEED
    // v = sqrt(MIN_SPEED^2 + 2 * accelConstant * distance)
    float accelPhase = sqrt(MIN_SPEED * MIN_SPEED + 2.0f * accelConstant * distanceTraveled);

    // Deceleration phase: ramps down toward MIN_SPEED as target approaches
    float remaining = pathLengthCounts - distanceTraveled;
    if (remaining < 0.0f) {
        remaining = 0.0f;
    }
    float decelPhase = sqrt(MIN_SPEED * MIN_SPEED + 2.0f * accelConstant * remaining);

    // Use the lowest ceiling at this point in the move
    float v = accelPhase;
    if (decelPhase < v) {
        v = decelPhase;
    }
    if (v > speed) {
        v = speed;  // Don't exceed commanded speed
    }
    return v;

}

// Check if it's time to print debug output
bool MotionController::debugTick() {
    unsigned long now = millis();
    if (now - lastDebugPrint >= DEBUG_PRINT_INTERVAL_MS) {
        lastDebugPrint = now;
        return true;
    }
    return false;
}

// Stop both motors immediately
void MotionController::Idle() {
    // Stop both motors
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

// Stop motors during homing 
void MotionController::HomingIdle(){
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void MotionController::HomingFunction() {

    homingRunning = true;
    switch (homingState) {
        case MOVE_TO_LEFT:
            // Move left until hitting left limit switch
            if (last_pressed == sB) {
                // Edge case: bottom switch hit first, move up slightly to clear it
                last_pressed = START;
                setTarget(0.0f, 5.0f, 75.0f);
                homingState = BOTTOM_EDGE_CASE_WAIT;
            }else if (last_pressed == sL) {
                // Left limit switch contacted
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_RIGHT;
            } else {
                // Move left at homing speed
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, homing_M1_Speed);
                analogWrite(enable2_pin, homing_M2_Speed);
            }
            break;

        case BOTTOM_EDGE_CASE_WAIT:
            // Wait for the offset movement to complete
            update();
            if (isCompleted()) {
                homingState = MOVE_TO_LEFT;
            }
            break;

        case MOVE_RIGHT:
            // Move right by fixed distance 
            setTarget(10.0f, 0.0f, 250.0f);
            homingState = WAIT_AFTER_RIGHT;
            break;

        case WAIT_AFTER_RIGHT:
            // Wait for movement to complete
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_LEFT2; 
            }
            break;

        case MOVE_TO_LEFT2:
            // Move back left slowly
            if (last_pressed == sL) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_RIGHT2;
            } else {
                // Move left at slower speed for accuracy
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, 98);
                analogWrite(enable2_pin, 75);
            }
            break;
       
        case MOVE_RIGHT2:
            // Move right by small amount
            setTarget(5.0f, 0.0f, 98.0f);
            homingState = WAIT_AFTER_RIGHT2;
            break;

        case WAIT_AFTER_RIGHT2:
            // Wait to complete
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_BOTTOM;  // Find bottom
            }
            break;

        case MOVE_TO_BOTTOM:
            // Move down until hitting bottom limit switch
            if (last_pressed == sB) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_UP;
            } else {
                // Move down at homing speed
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, homing_M1_Speed);
                analogWrite(enable2_pin, homing_M2_Speed);
            }
            break;

        case MOVE_UP:
            //Move up by fixed distance
            setTarget(0.0f, 10.0f, 250.0f);
            homingState = WAIT_AFTER_UP;
            break;
       
        case WAIT_AFTER_UP:
            // Wait for movement to complete
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = MOVE_TO_BOTTOM2;  // positioning for Y
            }
            break;
       
        case MOVE_TO_BOTTOM2:
            // Move back down slowly
            if (last_pressed == sB) {
                last_pressed = START;
                HomingIdle();
                homingState = MOVE_UP2;
            } else {
                // Move down at slower homing speed
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, 98);
                analogWrite(enable2_pin, 75);
            }
            break;

        case MOVE_UP2:
            // Move up by smaller amount
            setTarget(0.0f, 5.0f, 98.0f);
            homingState = WAIT_AFTER_UP2;
            break;
       
        case WAIT_AFTER_UP2:
            // Wait for positioning to complete
            update();
            if (isCompleted()) {
                HomingIdle();
                last_pressed = START;
                homingState = HOMING_COMPLETE;  // Move to origin
            }
            break;

        case HOMING_COMPLETE:
            update();

            if (isCompleted()) {
                // Homing done: reset counters
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

// Check if homing is completed
bool MotionController::isHomingComplete() const {
    return homingComplete;
}

// Initialise homing
void MotionController::StartHoming() {
    homingState = MOVE_TO_LEFT;    // Start from first stage
    homingComplete = false;        // Reset completion flag
    moving_completed = true;       // Can execute update() calls

    // Reset PI variables for homing
    integralMotor1 = 0;
    integralMotor2 = 0;
    prevIntegralMotor1 = 0;
    prevIntegralMotor2 = 0;
}