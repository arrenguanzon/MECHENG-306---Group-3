#include "encoder.h"
#include <Arduino.h> 

#define gantry_width 100
#define gantry_length 100

#define MM_PER_COUNT (15.0f * PI / 8256.0f) // 1 count = 5.71 um

Encoder::Encoder() {
    motor1Count = 0;
    motor2Count = 0;
    distancex = 0;
    distancey = 0;
}

long Encoder::convertToCounts(float distanceInMM) {
    // return (long)round(distanceInMM / MM_PER_COUNT);
    float counts = distanceInMM / MM_PER_COUNT;

    // Debugging Prints
    // Serial.print("Distance: ");
    // Serial.println(distanceInMM);

    // Serial.print("MM per count: ");
    // Serial.println(MM_PER_COUNT, 8);

    // Serial.print("Calculated counts: ");
    // Serial.println(counts);

    return (int)round(counts);
}

long Encoder::convertTodistance(float counts){
    float distance = counts*MM_PER_COUNT;

    return(int)round(distance);

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