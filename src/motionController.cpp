#include <Arduino.h>
#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int homing_M2_Speed = 192;
int homing_M1_Speed = 250;

// Maximum and minimum speed limits for motors
int MIN_SPEED = 85; // tuned
int MAX_SPEED = 180; // tuned

// PI Variables
// Controller gains (kp + ki > 0.05 for error = 100mm [~3000 encoder counts] to output min. speed of 80)
float Kp = 0.1f; // tune
float Ki = 0.05f; // tune
int positionTolerance = 100; //adjust based on testing

// Velocity profile acceleration constant (tune on the bench alongside Kp/Ki)
float accelConstant = 2.0f; // placeholder — tune

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

    Serial.println("=== setTarget() ===");
    Serial.print("targetX: ");
    Serial.println(targetX);
    Serial.print("targetY: ");
    Serial.println(targetY);

    calculateMotorTargets();

    Serial.print("targetMotor1: ");
    Serial.println(targetMotor1);
    Serial.print("targetMotor2: ");
    Serial.println(targetMotor2);
    
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

    float vPath = calculateVelocityCeiling(dPath);

    float ceiling1 = speed;
    float ceiling2 = speed;
    if (pathLengthCounts > 0.0f) {
        ceiling1 = vPath * (dist1Total / pathLengthCounts);
        ceiling2 = vPath * (dist2Total / pathLengthCounts);
        if (ceiling1 > MAX_SPEED) ceiling1 = MAX_SPEED;
        if (ceiling2 > MAX_SPEED) ceiling2 = MAX_SPEED;
    }

    Serial.print("dPath: ");
    Serial.print(dPath);
    Serial.print(" | vPath: ");
    Serial.print(vPath);
    Serial.print(" | ceiling1: ");
    Serial.print(ceiling1);
    Serial.print(" | ceiling2: ");
    Serial.println(ceiling2);

    // MOTOR 1 CONTROL //
    if (abs(motor1Error) <= positionTolerance)  {
        // Stop motor 1
        digitalWrite(motor1_pin, 0);
        analogWrite(enable1_pin, 0);

        // Reset integral terms
        integralMotor1 = 0;
        prevIntegralMotor1 = 0;
    } else {
        // Integral terms for PID control
        integralMotor1 = prevIntegralMotor1 + Ki * motor1Error;
        // Motor output
        float speedMotor1 = abs(Kp *   motor1Error + integralMotor1); // must be positive for analogWrite

        // Integral clamping to prevent windup
        if (speedMotor1 > ceiling1) {
            speedMotor1 = ceiling1;
            integralMotor1 = prevIntegralMotor1; // Clamp: DON'T update integral!
        } else {
            prevIntegralMotor1 = integralMotor1; // Only update previous integral if not saturated
        }
        if (speedMotor1 < MIN_SPEED) {
            speedMotor1 = MIN_SPEED;
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
        if (speedMotor2 < MIN_SPEED) {
            speedMotor2 = MIN_SPEED;
        }

        // MOTOR 2 CONTROL //
        digitalWrite(motor2_pin, motor2Error > 0 ? HIGH : LOW);
        analogWrite(enable2_pin, speedMotor2);
    }

    if (abs(motor1Error) <= positionTolerance && abs(motor2Error) <= positionTolerance) {
        updateAbsolutePosition();

        if(!homingRunning){
            Serial.println("=== Movement Complete ===");
            Serial.print("Absolute X: ");
            Serial.println(absoluteX);
            Serial.print("Absolute Y: ");
            Serial.println(absoluteY);
        }
        
        moving_completed = true;
        
    }

    Serial.print("M1 error: ");
    Serial.print(motor1Error);
    Serial.print(" | M2 error: ");
    Serial.print(motor2Error);
    Serial.print(" | Complete: ");
    Serial.println(moving_completed);
}



bool MotionController::isCompleted() const {
    return moving_completed;
}

void MotionController::calculateMotorTargets() {
    long motor1Counts = encoder.convertToCounts(targetX - targetY);
    long motor2Counts = encoder.convertToCounts(targetX + targetY);

    Serial.println("=== calculateMotorTargets() ===");

    Serial.print("motor1Counts: ");
    Serial.println(motor1Counts);

    Serial.print("motor2Counts: ");
    Serial.println(motor2Counts);

    // Snapshot starting position for this move (velocity profile reference point)
    startMotor1 = encoder.getMotor1Count();
    startMotor2 = encoder.getMotor2Count();

    Serial.print("Current M1: ");
    Serial.println(startMotor1);

    Serial.print("Current M2: ");
    Serial.println(startMotor2);

    // Convert target positions to encoder counts
    targetMotor1 = startMotor1 + motor1Counts;
    targetMotor2 = startMotor2 + motor2Counts;

    // Velocity profile: per-motor distance and true Cartesian path length for this move
    dist1Total = abs((float)motor1Counts);
    dist2Total = abs((float)motor2Counts);
    pathLengthCounts = encoder.convertToCounts(sqrt(targetX * targetX + targetY * targetY));

    Serial.print("Path length (counts): ");
    Serial.println(pathLengthCounts);

}

void MotionController::updateAbsolutePosition() {
    absoluteX += targetX;
    absoluteY += targetY;

    if(!homingRunning){
        Serial.println("=== Absolute Position Updated ===");
        Serial.print("Absolute X: ");
        Serial.println(absoluteX);
        Serial.print("Absolute Y: ");
        Serial.println(absoluteY);
    }
}

float MotionController::calculateVelocityCeiling(float distanceTraveled) const {
    float v = sqrt(MIN_SPEED * MIN_SPEED + 2.0f * accelConstant * distanceTraveled);
        if (v > speed) {
            v = speed;
        }
        return v;
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
    Serial.print("Homing state: ");
    Serial.print(homingState);
    Serial.print(" | last_pressed: ");
    Serial.println(last_pressed);
    homingRunning = true;

    // Homing state machine
    switch (homingState) {
        case MOVE_TO_LEFT:
            if (last_pressed == sB) {
                last_pressed = START;
                setTarget(0.0f, 5.0f, 80.0f);
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
                analogWrite(enable1_pin, 104);
                analogWrite(enable2_pin, 80);
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
                analogWrite(enable1_pin, 104);
                analogWrite(enable2_pin, 80);
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
