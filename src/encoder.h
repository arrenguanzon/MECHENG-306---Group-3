#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// 1 count = 9.513mm

class Encoder {

    private:
        int motor1Count;
        int motor2Count;
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