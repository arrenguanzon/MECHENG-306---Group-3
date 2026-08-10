#ifndef FSM_H
#define FSM_H

#include "gcode.h"

class FSM {
    public:
        enum State
        {
            IDLE,
            HOMING,
            MOVING,
            FAULT
        };

    private:
        State state;

    public:
        FSM();
        State getState() const;
        void update(const GCode& gcode);
        void setState(State newState);
        const char* getStateName() const;
};

#endif