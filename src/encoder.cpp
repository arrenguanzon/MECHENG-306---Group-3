#include "encoder.h"
#include <Arduino.h> 

#define MM_PER_COUNT (14.3f * PI / 8256.0f) // 1 count = 5.71 um

Encoder::Encoder() {
    motor1Count = 0;
    motor2Count = 0;
}

long Encoder::convertToCounts(float distanceInMM) {
    float counts = distanceInMM / MM_PER_COUNT;
    return (long)round(counts);
}

long Encoder::getMotor1Count() {
    return motor1Count;
}

long Encoder::getMotor2Count() {
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