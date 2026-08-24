#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <math.h>

class Encoder {

    private:
        volatile long motor1Count;
        volatile long motor2Count;

    public:
        Encoder();
        long convertToCounts(float distanceInMM);
        long convertToDistance(float counts);
        long getMotor1Count();
        long getMotor2Count();
        void incrementMotor1Count();
        void incrementMotor2Count();
        void decrementMotor1Count();
        void decrementMotor2Count();
        void resetCounts();
};

#endif