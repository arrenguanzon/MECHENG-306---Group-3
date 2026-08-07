#include "encoder.h"
#include <Arduino.h> 
#include <motion.h>

// Set up encoder pins
#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21

#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7

#define gantry_width 100
#define gantry_length 100

encoder::encoder()
{
    currentPos = 0;
    destinationPos = 0;
    currentCountA = 0;
    currentCountB = 0;
    distanceFromOriginx = 0; //don't know if this will work should probably do something in homing for this
    distanceFromOriginy = 0; //don't know if this will work
}

int encoder::convertToCounts(int mm)
{
    return mm * 9.513;
}



int encoder::moveTo(int x, int y)
{
    int xCounts = 0;
    xCounts = convertToCounts(x);
    int yCounts = 0;
    yCounts = convertToCounts(y);

    if ((distanceFromOriginx + xCounts) > gantry_length || (distanceFromOriginy + yCounts) > gantry_width) {
        Serial.println("Error: Destination exceeds gantry limits.");
        return -1;
    }

    if (currentCountA < xCounts) {
        motion::verticalUp(100);
    } 
    else if (currentCountA > xCounts) {
        motion::verticalDown(100);
    } 
    else{
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
    }

    if (currentCountB < yCounts) {
       motion::horizontalRight(100);
    } 
    else if (currentCountB > yCounts) { 
       motion::horizontalLeft(100);
    } 
    else {
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
    }


    return 0;
}

//need to double check direction I've got no idea whether this logc will work


