#include "encoder.h"
#include <Arduino.h> 

// Set up encoder pins
#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21

#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7

encoder::encoder()
{
    currentPos = 0;
    destinationPos = 0;
    currentCountA = 0;
    currentCountB = 0;
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


    if (currentCountA < xCounts)
    {
        // move motors forward
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, 100);
        analogWrite(enable2_pin, 100);
    }
    else if (currentCountA > xCounts)
    {
        // move motors backward
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, HIGH);
        analogWrite(enable1_pin, 100);
        analogWrite(enable2_pin, 100);
    }
    else
    {
        // stop motors
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
    }

    if (currentCountB < yCounts)
    {
        // move motors forward
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, 100);
        analogWrite(enable2_pin, 100);
    }
    else if (currentCountB > yCounts)
    {
        // move motors backward
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, HIGH);
        analogWrite(enable1_pin, 100);
        analogWrite(enable2_pin, 100);
    }
    else
    {
        // stop motors
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
    }


    return 0;
}

//need to double check direction I've got no idea whether this logc will work


