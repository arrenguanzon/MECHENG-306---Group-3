// Encoder counter management for motor position tracking
// Manages quadrature encoder counting for both motors
// Converts between encoder counts and physical distance (mm)

#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <math.h>

class Encoder {

    private:
        volatile long motor1Count;   // Cumulative encoder count for motor 1
        volatile long motor2Count;   // Cumulative encoder count for motor 2
    public:
        // Initialise encoder counters to zero
        Encoder();
        
        // Convert physical distance in millimeters to encoder counts returns equivalent encoder count value
        long convertToCounts(float distanceInMM);
        
        // Get current encoder count for motor 1
        long getMotor1Count();
        
        // Get current encoder count for motor 2
        long getMotor2Count();
        
        // Increment motor 1 encoder counter
        void incrementMotor1Count();
        
        // Increment motor 2 encoder counter
        void incrementMotor2Count();
        
        // Decrement motor 1 encoder counter
        void decrementMotor1Count();
        
        // Decrement motor 2 encoder counter
        void decrementMotor2Count();
        
        // Reset both encoder counters to zero
        void resetCounts();
};

#endif