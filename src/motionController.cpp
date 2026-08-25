#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;
#define MIN_SPEED 70
#define MAX_SPEED 100
#define POSITION_TOLERANCE 800 // Tolerance in encoder counts for position control

// PI Variables
// Controller gains (kp + ki > 0.05 for error = 100mm [~3000 encoder counts] to output min. speed of 75)
float Kp = 0.05f; // tune
float Ki = 0.00f; // tune
int positionTolerance = 800; //adjust based on testing

MotionController::MotionController(Encoder& encoder, float& absoluteX, float& absoluteY, const volatile SwitchState& last_pressed) : encoder(encoder), absoluteX(absoluteX), absoluteY(absoluteY), last_pressed(last_pressed) {
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
        if (speedMotor1 > MAX_SPEED) {
            speedMotor1 = MAX_SPEED;
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
        if (speedMotor2 > MAX_SPEED) {
            speedMotor2 = MAX_SPEED;
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
    switch (homingState) {
        case MOVE_TO_LEFT:
            if (last_pressed == sB) {
            //if (last_pressed == sB) {
                setTarget(0.0f, 5.0f, 50.0f);
                homingState = BOTTOM_EDGE_CASE_WAIT;
            } else if (last_pressed == sT) {
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, homing_M1_Speed);
                analogWrite(enable2_pin, homing_M2_Speed);
                setTarget(0.0f, -5.0f, 50.0f);
                homingState = TOP_EDGE_CASE_WAIT;
            }else if (last_pressed == sL) {
            //} else if (last_pressed == sL) {
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
        
        case TOP_EDGE_CASE_WAIT:
            update();
            if (isCompleted()) {
                homingState = MOVE_TO_LEFT;
            }
            break;

        case MOVE_RIGHT:
            digitalWrite(motor1_pin, HIGH);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, homing_M1_Speed);
            analogWrite(enable2_pin, homing_M2_Speed);
            setTarget(5.0f, 0.0f, 50.0f);
            homingState = WAIT_AFTER_RIGHT;
            break;

        case WAIT_AFTER_RIGHT:
            update();
            if (isCompleted()) {
                HomingIdle();
                homingState = MOVE_TO_BOTTOM;
            }
            break;

        case MOVE_TO_BOTTOM:
            if (last_pressed == sB) {
            //if (last_pressed == sB) {
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
            digitalWrite(motor1_pin, LOW);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, homing_M1_Speed);
            analogWrite(enable2_pin, homing_M2_Speed);
            setTarget(0.0f, 5.0f, 50.0f);
            homingState = HOMING_COMPLETE;
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
}

