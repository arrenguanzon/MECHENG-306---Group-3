#include "motionController.h"

// Set up motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;
#define MIN_SPEED 40
#define MAX_SPEED 100
#define SLOW_ZONE 500
#define POSITION_TOLERANCE 100 // Tolerance in encoder counts for position control

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

void MotionController::update() { // This controls the motors
    Serial.println("=== update() ===");

    if (completed) {
        Serial.println("Motion already completed");
        return;
    }

    // Get current encoder positions
    long current1 = encoder.getMotor1Count();
    long current2 = encoder.getMotor2Count();

    // Calculate position errors
    long error1 = targetMotor1 - current1;
    long error2 = targetMotor2 - current2;

    Serial.print("M1: ");
    Serial.print(current1);
    Serial.print(" / ");
    Serial.print(targetMotor1);

    Serial.print("    M2: ");
    Serial.print(current2);
    Serial.print(" / ");
    Serial.println(targetMotor2);

    // MOTOR 1
    int motor1Speed;

    if (abs(error1) <= POSITION_TOLERANCE) {
        // Close enough to target
        motor1Speed = 0;
    }
    else if (abs(error1) < SLOW_ZONE) {
        // Slow down as we approach target
        motor1Speed = map(
            abs(error1),
            0,
            SLOW_ZONE,
            MIN_SPEED,
            MAX_SPEED
        );
    }
    else {
        // Far from target
        motor1Speed = MAX_SPEED;
    }

    // Motor 1 direction
    //
    // HIGH -> positive encoder counts
    // LOW  -> negative encoder counts
    if (error1 > 0) {
        digitalWrite(motor1_pin, HIGH);
    }
    else if (error1 < 0) {
        digitalWrite(motor1_pin, LOW);
    }
    analogWrite(enable1_pin, motor1Speed);

    // MOTOR 2
    int motor2Speed;

    if (abs(error2) <= POSITION_TOLERANCE) {
        // Close enough to target
        motor2Speed = 0;
    }
    else if (abs(error2) < SLOW_ZONE) {
        // Slow down as we approach target
        motor2Speed = map(
            abs(error2),
            0,
            SLOW_ZONE,
            MIN_SPEED,
            MAX_SPEED
        );
    }
    else {
        // Far from target
        motor2Speed = MAX_SPEED;
    }

    // Motor 2 direction
    //
    // HIGH -> positive encoder counts
    // LOW  -> negative encoder counts
    if (error2 > 0) {
        digitalWrite(motor2_pin, HIGH);
    }
    else if (error2 < 0) {
        digitalWrite(motor2_pin, LOW);
    }

    analogWrite(enable2_pin, motor2Speed);

    // TARGET REACHED //
    if (abs(error1) <= POSITION_TOLERANCE &&
        abs(error2) <= POSITION_TOLERANCE) {
        Serial.println("=== TARGET REACHED ===");
        // Stop both motors
        analogWrite(enable1_pin, 0);
        analogWrite(enable2_pin, 0);

        completed = true;
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


// State functions
void MotionController::Idle() {
    // Stop both motors
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void MotionController::Homing(const volatile SwitchState& switchState){
    int a = 0;
    while(switchState != MotionController::sL){
        if(switchState == MotionController::sB && a == 0){
            digitalWrite(motor1_pin, LOW);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, M1_speed);
            analogWrite(enable2_pin, M2_speed); 
            delay(1000);
            a = 1;
        }
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);

    }
    
    
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(500);

    // implement logic to move to the right
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, M1_speed);
    analogWrite(enable2_pin, M2_speed);
    delay(500);
    
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(500);

    // while((switchState == MotionController::START) || (switchState == MotionController::sT) || (switchState == MotionController::sL) || (switchState == MotionController::sR)){
    while(switchState != MotionController::sB){
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);

    }
    
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(500);
    
    digitalWrite(motor1_pin, LOW);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, M1_speed);
    analogWrite(enable2_pin, M2_speed);
    delay(500);
    
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(500);

    absoluteX = 0.0f;
    absoluteY = 0.0f;
    encoder.resetCounts();
}