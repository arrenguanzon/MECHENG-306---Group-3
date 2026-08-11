#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <string.h>
#include <motion.h>

// 1 count = 9.513mm

class encoder {

    public:
    encoder();
    int convertToCounts(int mm);
    int move(int x, int y);
    volatile int currentCountA = 0;
    volatile int currentCountB = 0;
    volatile int distanceFromOriginx;
    volatile int distanceFromOriginy;

    private:
    int currentPos;
    int destinationPos;

   


};

#endif