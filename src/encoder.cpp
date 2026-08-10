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


