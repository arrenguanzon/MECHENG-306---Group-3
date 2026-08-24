#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Maximum and minimum speed limits for motors
int MAX_SPEED = 100; // Change to actual value

// Controller gains (kp + ki > 0.05 for error = 100mm [~3000 encoder counts] to output min. speed of 75)
float Kp = 0.05f; // tune
float Ki = 0.00f; // tune

// Position tolerance (acceptable error in encoder counts)
int positionTolerance = 800; //adjust based on testing

MotionController::MotionController(Encoder& encoder, float& absoluteX, float& absoluteY) : encoder(encoder), absoluteX(absoluteX), absoluteY(absoluteY) {
    targetX = 0.0f;
    targetY = 0.0f;
    speed = 0.0f;

    targetMotor1 = 0;
    targetMotor2 = 0;

    prevIntegralMotor1 = 0;
    prevIntegralMotor2 = 0;

    integralMotor1 = 0;
    integralMotor2 = 0;

    completed = true;
}

void MotionController::setTarget(float x, float y, float speed) {
    targetX = x;
    targetY = y;
    this->speed = speed;
    calculateMotorTargets();

    // PRINTING /////////////////////////////////////////////////////////////////
    Serial.println("TARGET MOTOR COUNTS [relative to current position when called]");
    Serial.print("Target Motor 1 Counts: ");
    Serial.println(targetMotor1);
    Serial.print("Target Motor 2 Counts: ");
    Serial.println(targetMotor2);
    Serial.println(" ");
    /////////////////////////////////////////////////////////////////////////////
    
    completed = false;
}

void MotionController::update() {
    if (completed) {
        Serial.println("Motion completed");
        return;
    }

    // Update current motor positions (these are relative to the position when motion was called; starts at 0 and increases to target)
    long currentMotor1 = encoder.getMotor1Count();
    long currentMotor2 = encoder.getMotor2Count();

    ////////////////////////////////////////////////////////
    Serial.println("CURRENT MOTOR POSITION [relative to position when command was called; increases from 0 to target]");
    Serial.print("Current motor 1: ");
    Serial.println(currentMotor1);
    Serial.print("Current motor 2: ");
    Serial.println(currentMotor2);
    Serial.println("  ");
    ////////////////////////////////////////////////////////

    // Calculate errors (direction of motion is determined by the sign of the error)
    long motor1Error = targetMotor1 - currentMotor1;
    long motor2Error = targetMotor2 - currentMotor2;

    ////////////////////////////////////////////////////////
    Serial.println("MOTOR ERRORS [should decrease to 100]"); // (decreases until 100 [position tolerance])
    Serial.print("Motor 1 error: ");
    Serial.println(motor1Error);
    Serial.print("Motor 2 error: ");
    Serial.println(motor2Error);
    Serial.println("  ");
    ////////////////////////////////////////////////////////

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

        ////////////////////////////////////////////////////////
        Serial.println("MOTOR 1 SPEED [max of 100; should decrease the end]"); 
        Serial.print("Motor 1 speed: ");
        Serial.println(speedMotor1);
        ////////////////////////////////////////////////////////

        // MOTOR 1 CONTROL //
        digitalWrite(motor1_pin, motor1Error > 0 ? HIGH : LOW);
        analogWrite(enable1_pin, speedMotor1);
    }

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

        ////////////////////////////////////////////////////////
        Serial.println("MOTOR 2 SPEED [max of 100; should decrease the end]"); 
        Serial.print("Motor 2 speed: ");
        Serial.println(speedMotor2);
        Serial.println(" ");
        ////////////////////////////////////////////////////////

        // MOTOR 2 CONTROL //
        digitalWrite(motor2_pin, motor2Error > 0 ? HIGH : LOW);
        analogWrite(enable2_pin, speedMotor2);
    }

    if (abs(motor1Error) <= positionTolerance && abs(motor2Error) <= positionTolerance) {
        completed = true;
        return;
    }
}

bool MotionController::isCompleted() const {
    return completed;
}

void MotionController::calculateMotorTargets() {
    long motor1Counts = encoder.convertToCounts(targetX - targetY);
    long motor2Counts = encoder.convertToCounts(targetX + targetY);

    // Convert target positions to encoder counts
    targetMotor1 = encoder.getMotor1Count() + motor1Counts;
    targetMotor2 = encoder.getMotor2Count() + motor2Counts;
}

void MotionController::Idle() {
    // Stop both motors
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void MotionController::Homing(const volatile SwitchState& switchState){
   // Integrate Homing here
}
