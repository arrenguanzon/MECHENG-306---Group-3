#ifndef FSM_H
#define FSM_H

#include "gcode.h"

class FSM
{
public:

    enum State
    {
        IDLE,
        HOMING,
        MOVING,
        FAULT
    };

    FSM();

    void update();

private:

    State state;

    void updateIdle();
    void updateHoming();
    void updateMoving();
    void updateFault();
};

#endif