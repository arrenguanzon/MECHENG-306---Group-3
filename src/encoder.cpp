#include "encoder.h"
#include <Arduino.h> 

#define gantry_width 100
#define gantry_length 100

#define MM_PER_COUNT 15*PI/8256 // 1 count = 5.71 um

Encoder::Encoder() {
    motor1Count = 0;
    motor2Count = 0;
}

int Encoder::convertToCounts(float distanceInMM) {
    return (distanceInMM / MM_PER_COUNT);
}

int Encoder::getMotor1Count() {
    return motor1Count;
}

int Encoder::getMotor2Count() {
    return motor2Count;
}

void Encoder::incrementMotor1Count() {
    motor1Count++;
}

void Encoder::incrementMotor2Count() {
    motor2Count++;
}

void Encoder::decrementMotor1Count() {
    motor1Count--;
}

void Encoder::decrementMotor2Count() {
    motor2Count--;
}

void Encoder::resetCounts() {
    motor1Count = 0;
    motor2Count = 0;
}
