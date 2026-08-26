#include <Arduino.h>
#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int homing_M2_Speed = 100;
int homing_M1_Speed = 130;

// Maximum and minimum speed limits for motors
int MIN_SPEED = 75; // tuned
int MAX_SPEED = 200; // tuned

// Acceleration rate for trapezoidal velocity profile
float ACCELERATION_RATE = 0.04f; // PWM units per millisecond (e.g., 0.04 PWM/ms = 0.75s to go from 70 PWM to 100 PWM)

// Controller gains
float Kp = 0.05f; // TUNE FIRST: eliminate tracking error
float Ki = 0.00f; // TUNE SECOND: eliminate steady state error
float Kv = 0.8f; // does majority of the heavy lifting for motor power

// Position tolerance (acceptable error in encoder counts)
int positionTolerance = 800; // TUNE: can be much lower

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
    
    this->speed = constrain(speed, MIN_SPEED, MAX_SPEED); // constrain speed to be within min and max limits
    this->startSpeed = MIN_SPEED; // start at minimum speed
    this->accelRate = ACCELERATION_RATE; // set acceleration rate
    this->moveStartTime = millis();

    Serial.println("=== setTarget() ===");
    Serial.print("targetX: ");
    Serial.println(targetX);
    Serial.print("targetY: ");
    Serial.println(targetY);
    Serial.print("speed: ");
    Serial.println(this->speed);

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

    //// TRAPEZOIDAL VELOCITY PROFILE ////////////////
    // Gradually increase motor velocity over time (acceleration ramp)
    unsigned long elapsedTime = millis() - moveStartTime;
    float accelerationSpeed = startSpeed + (accelRate * (float)elapsedTime); 
    
    // Deceleration ramp
    long maxError = max(abs(motor1Error), abs(motor2Error));
    float decelerationSpeed = sqrt(2.0f * accelRate * (float)maxError * (15.0f * PI / 8256.0f)); // deceleration speed = sqrt(2 * acceleration * error distance)

    // Trapezoidal Constraint (Take lowest speed of acceleration, cruise, and deceleration)
    float currentRampedMaxSpeed = min(accelerationSpeed, (float)speed); // once cruise speed is reached, acceleration speed increases so CRUISE SPEED will be the smallest value
    currentRampedMaxSpeed = min(currentRampedMaxSpeed, decelerationSpeed); // as error decreases, DECELERATION SPEED will become smaller than cruise control

    // If the velocity profile motor speed is too low (and position tolerance isn't reached), use minimum speed
    if (currentRampedMaxSpeed < MIN_SPEED && maxError > positionTolerance) {
        currentRampedMaxSpeed = MIN_SPEED;
    }


    //// MOTOR 1 /////////////////////////////////////
    if (abs(motor1Error) <= positionTolerance)  {
        // Stop motor 1
        analogWrite(enable1_pin, 0);

        // Reset integral terms
        integralMotor1 = 0;
        prevIntegralMotor1 = 0;
    } else {
        // Integral terms for PID control
        integralMotor1 = prevIntegralMotor1 + Ki * motor1Error;
        // Motor output
         float speedMotor1 = abs(Kp * (float)motor1Error + integralMotor1 + Kv * currentRampedMaxSpeed); // must be positive for analogWrite

        // Integral clamping to prevent windup
        if (speedMotor1 > currentRampedMaxSpeed) {
            speedMotor1 = currentRampedMaxSpeed; 
            integralMotor1 = prevIntegralMotor1; // Clamp: don't update integral
        } else {
            prevIntegralMotor1 = integralMotor1; // Only update previous integral if not saturated
        }

        // MOTOR 1 CONTROL
        digitalWrite(motor1_pin, motor1Error > 0 ? HIGH : LOW);
        analogWrite(enable1_pin, speedMotor1);
    }

    //// MOTOR 2 /////////////////////////////////////
    if (abs(motor2Error) <= positionTolerance) {
        // Stop motor 2
        analogWrite(enable2_pin, 0);

        // Reset integral terms
        integralMotor2 = 0;
        prevIntegralMotor2 = 0;
    } else {
        // Integral terms for PID control
        integralMotor2 = prevIntegralMotor2 + Ki * motor2Error;

        // Calculate motor speed outputs
        float speedMotor2 = abs(Kp * (float)motor2Error + integralMotor2 + Kv * currentRampedMaxSpeed); // must be positive for analogWrite

        // Integral clamping to prevent windup
        if (speedMotor2 > currentRampedMaxSpeed) {
            speedMotor2 = currentRampedMaxSpeed; // motor speed never goes above MAX_SPEED
            integralMotor2 = prevIntegralMotor2; // Clamp: don't update integral
        } else {
            prevIntegralMotor2 = integralMotor2; // Only update previous integral if not saturated
        }

        // MOTOR 2 CONTROL 
        digitalWrite(motor2_pin, motor2Error > 0 ? HIGH : LOW);
        analogWrite(enable2_pin, speedMotor2);
    }

    if (abs(motor1Error) <= positionTolerance && abs(motor2Error) <= positionTolerance) {
        updateAbsolutePosition();

        Serial.println("=== Movement Complete ===");
        Serial.print("Absolute X: ");
        Serial.println(absoluteX);
        Serial.print("Absolute Y: ");
        Serial.println(absoluteY);

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

    Serial.print("Current M1: ");
    Serial.println(encoder.getMotor1Count());

    Serial.print("Current M2: ");
    Serial.println(encoder.getMotor2Count());

    // Convert target positions to encoder counts
    targetMotor1 = encoder.getMotor1Count() + motor1Counts;
    targetMotor2 = encoder.getMotor2Count() + motor2Counts;

}

void MotionController::updateAbsolutePosition() {
    absoluteX += targetX;
    absoluteY += targetY;

    Serial.println("=== Absolute Position Updated ===");
    Serial.print("Absolute X: ");
    Serial.println(absoluteX);
    Serial.print("Absolute Y: ");
    Serial.println(absoluteY);
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
    // Homing state machine
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
