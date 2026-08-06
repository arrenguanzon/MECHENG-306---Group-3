#ifndef MOTION_H
#define MOTION_H

#include <Arduino.h>
#include <string.h>


class motion {
    public:
    motion();
    static void horizontalRight(int speed);
    static void horizontalLeft(int speed);
    static void verticalUp(int speed);
    static void verticalDown(int speed);

};

#endif