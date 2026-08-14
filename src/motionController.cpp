#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;

// Maximum and minimum speed limits for motors
int MAX_SPEED = 100; // Change to actual value

// Controller gains (kp + ki > 0.05 for error = 100mm [~3000 encoder counts] to output min. speed of 75)
float Kp = 0.05f; // tune
float Ki = 0.00f; // tune

// Position tolerance (acceptable error in encoder counts)
int positionTolerance = 100; //adjust based on testing

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
    
    completed = false;
}

void MotionController::update() {
    if (completed) {
        return;
    }

    // Update current motor positions
    int currentMotor1 = encoder.getMotor1Count();
    int currentMotor2 = encoder.getMotor2Count();

    // Calculate errors
    int motor1Error = targetMotor1 - currentMotor1;
    int motor2Error = targetMotor2 - currentMotor2;

    if (abs(motor1Error) <= positionTolerance)  {
        // Stop motor 1
        digitalWrite(motor1_pin, 0);
        analogWrite(enable1_pin, 0);

        // Reset integral terms
        integralMotor1 = 0;
        prevIntegralMotor1 = 0;
    } else {
        // Integral terms for PID control
        integralMotor1 = prevIntegralMotor1 + Ki * abs(motor1Error);
        // Motor output
        float speedMotor1 = Kp * abs(motor1Error) + integralMotor1;

        // Integral clamping to prevent windup
        if (speedMotor1 > MAX_SPEED) {
            speedMotor1 = MAX_SPEED;
            integralMotor1 = prevIntegralMotor1; // Clamp: DON'T update integral!
        } else {
            prevIntegralMotor1 = integralMotor1; // Only update previous integral if not saturated
        }

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
        integralMotor2 = prevIntegralMotor2 + Ki * abs(motor2Error);

        // Calculate motor speed outputs
        float speedMotor2 = Kp * abs(motor2Error) + integralMotor2;

        // Integral clamping to prevent windup
        if (speedMotor2 > MAX_SPEED) {
            speedMotor2 = MAX_SPEED;
            integralMotor2 = prevIntegralMotor2; // Clamp: DON'T update integral!
        } else {
            prevIntegralMotor2 = integralMotor2; // Only update previous integral if not saturated
        }

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
    int motor1Counts = encoder.convertToCounts(targetX - targetY);
    int motor2Counts = encoder.convertToCounts(targetX + targetY);
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
    while((switchState == MotionController::START) || (switchState == MotionController::sT) || (switchState == MotionController::sB) || (switchState == MotionController::sR)){
        if(switchState == MotionController::sB){
            digitalWrite(motor1_pin, LOW);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, M1_speed);
            analogWrite(enable2_pin, M2_speed); 
            delay(1000);
        }
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);

    }
    
    Idle();
    // implement logic to move to the right
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, M1_speed);
    analogWrite(enable2_pin, M2_speed);
    delay(500);
    Idle();

    while((switchState == MotionController::START) || (switchState == MotionController::sT) || (switchState == MotionController::sL) || (switchState == MotionController::sR)){
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);
    }
    Idle();
    absoluteX = 0.0f;
    absoluteY = 0.0f;
    encoder.resetCounts();
}