#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;

#define POSITION_TOLERANCE 10 // Tolerance in encoder counts for position control

MotionController::MotionController(Encoder& encoder, float& absoluteX, float& absoluteY) : encoder(encoder), absoluteX(absoluteX), absoluteY(absoluteY) {
    targetX = 0.0f;
    targetY = 0.0f;
    speed = 0.0f;

    targetMotor1 = 0;
    targetMotor2 = 0;
    completed = true;
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
    
    completed = false;
}

void MotionController::update() {
    Serial.println("=== update() ===");

    if (completed) {
        Serial.println("Motion already completed");
        return;
    }
    long currentMotor1 = encoder.getMotor1Count();
    long currentMotor2 = encoder.getMotor2Count();

    Serial.print("M1: ");
    Serial.print(currentMotor1);
    Serial.print(" / ");
    Serial.println(targetMotor1);

    Serial.print("M2: ");
    Serial.print(currentMotor2);
    Serial.print(" / ");
    Serial.println(targetMotor2);

    // MOTOR 1 CONTROL //
    if (currentMotor1 < targetMotor1) {
        Serial.println("M1: FORWARD");
        // Move motor 1 forward
        digitalWrite(motor1_pin, HIGH);
        analogWrite(enable1_pin, speed);
    } else if (currentMotor1 > targetMotor1) {
        Serial.println("M1: BACKWARD");
        // Move motor 1 backward
        digitalWrite(motor1_pin, LOW);
        analogWrite(enable1_pin, speed);
    } else {
        Serial.println("M1: STOP");
        // Stop motor 1
        analogWrite(enable1_pin, 0);
    }

    // MOTOR 2 CONTROL //
    if (currentMotor2 < targetMotor2) {
        // Move motor 2 forward
        digitalWrite(motor2_pin, HIGH);
        analogWrite(enable2_pin, speed);
    } else if (currentMotor2 > targetMotor2) {
        // Move motor 2 backward
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable2_pin, speed);
    } else {
        // Stop motor 2
        analogWrite(enable2_pin, 0);
    }

    // Check if both motors have reached their targets //
    if (currentMotor1 == targetMotor1 && currentMotor2 == targetMotor2) {
        Serial.println("=== TARGET REACHED ===");
        completed = true;
        // Stop both motors
        analogWrite(enable1_pin, 0);
        analogWrite(enable2_pin, 0);

    }
}

bool MotionController::isCompleted() const {
    return completed;
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

    Serial.print("Target M1: ");
    Serial.println(targetMotor1);

    Serial.print("Target M2: ");
    Serial.println(targetMotor2);
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
#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;

MotionController::MotionController(Encoder& encoder, float& absoluteX, float& absoluteY) : encoder(encoder), absoluteX(absoluteX), absoluteY(absoluteY) {
    targetX = 0.0f;
    targetY = 0.0f;
    speed = 0.0f;

    targetMotor1 = 0;
    targetMotor2 = 0;
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
    int currentMotor1 = encoder.getMotor1Count();
    int currentMotor2 = encoder.getMotor2Count();

    // MOTOR 1 CONTROL //
    if (currentMotor1 < targetMotor1) {
        // Move motor 1 forward
        digitalWrite(motor1_pin, HIGH);
        analogWrite(enable1_pin, speed);
    } else if (currentMotor1 > targetMotor1) {
        // Move motor 1 backward
        digitalWrite(motor1_pin, LOW);
        analogWrite(enable1_pin, speed);
    } else {
        // Stop motor 1
        analogWrite(enable1_pin, 0);
    }

    // MOTOR 2 CONTROL //
    if (currentMotor2 < targetMotor2) {
        // Move motor 2 forward
        digitalWrite(motor2_pin, HIGH);
        analogWrite(enable2_pin, speed);
    } else if (currentMotor2 > targetMotor2) {
        // Move motor 2 backward
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable2_pin, speed);
    } else {
        // Stop motor 2
        analogWrite(enable2_pin, 0);
    }

    // Check if both motors have reached their targets //
    if (currentMotor1 == targetMotor1 && currentMotor2 == targetMotor2) {
        completed = true;
        // Stop both motors
        analogWrite(enable1_pin, 0);
        analogWrite(enable2_pin, 0);

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