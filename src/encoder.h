#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// 1 count = 9.513mm

class Encoder {

    public:
    encoder();
    int convertToCounts(int mm);
    int move(int x, int y);
    volatile int currentCountA = 0;
    volatile int currentCountB = 0;
    volatile int distanceFromOriginx;
    volatile int distanceFromOriginy;

    private:
        volatile int motor1Count;
        volatile int motor2Count;
    public:
        Encoder();
        int convertToCounts(float distanceInMM);
        int getMotor1Count();
        int getMotor2Count();
        void incrementMotor1Count();
        void incrementMotor2Count();
        void decrementMotor1Count();
        void decrementMotor2Count();
        void resetCounts();
};

#endif