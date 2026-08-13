#include "encoder.h"
#include <Arduino.h> 
#include <motion.h>

// Set up encoder pins
#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21

#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

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


int encoder::move(int x, int y)
{
    int xCounts = 0;
    xCounts = convertToCounts(x);
    int yCounts = 0;
    yCounts = convertToCounts(y);
    
    currentCountA = 0;
    currentCountB = 0;

    if (((distanceFromOriginx + x) > gantry_length || (distanceFromOriginy + y) > gantry_width) || ((distanceFromOriginx + x) < 0 || (distanceFromOriginy + y) < 0)){
        Serial.println("Error: Destination exceeds gantry limits.");
        return -1;
    }

    while (currentCountA < xCounts) {
        //motion::verticalUp(100);
        Serial.print("currentCountA: ");
        Serial.println(currentCountA);
    } 
    while(currentCountA > xCounts) {
        //motion::verticalDown(100);
        Serial.print("currentCountA: ");
        Serial.println(currentCountA);
    } 

    while (currentCountB < yCounts) {
       //motion::horizontalRight(100);
       Serial.print("currentCountB: ");
        Serial.println(currentCountB);
    } 

    while(currentCountB > yCounts) { 
       //motion::horizontalLeft(100);
       Serial.print("currentCountB: ");
       Serial.println(currentCountB);
    } 

    
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);


    return 0;
}

//need to double check direction I've got no idea whether this logc will work


