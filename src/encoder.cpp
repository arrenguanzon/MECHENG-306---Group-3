#include "encoder.h"
#include <Arduino.h> 

// Encoder resolution: pulses per revolution and wheel diameter
// 1 count = 5.71 um (micrometers)
#define MM_PER_COUNT (14.3f * PI / 8256.0f)

// Initialise encoder counters to zero
Encoder::Encoder() {
    motor1Count = 0;
    motor2Count = 0;
}

// Convert physical distance (mm) to encoder counts
long Encoder::convertToCounts(float distanceInMM) {
    float counts = distanceInMM / MM_PER_COUNT;
    return (long)round(counts);
}

// Get current count for motor 1
long Encoder::getMotor1Count() {
    return motor1Count;
}

// Get current count for motor 2
long Encoder::getMotor2Count() {
    return motor2Count;
}

// Increment motor 1 counter (called by ISR)
void Encoder::incrementMotor1Count() {
    motor1Count++;
}

// Increment motor 2 counter (called by ISR)
void Encoder::incrementMotor2Count() {
    motor2Count++;
}

// Decrement motor 1 counter (called by ISR)
void Encoder::decrementMotor1Count() {
    motor1Count--;
}

// Decrement motor 2 counter (called by ISR)
void Encoder::decrementMotor2Count() {
    motor2Count--;
}

// Reset both counters to zero (used at start of movement or after homing)
void Encoder::resetCounts() {
    motor1Count = 0;
    motor2Count = 0;
}