#ifndef FSM_H
#define FSM_H

#include "gcode.h"
#include "motionController.h"

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
        State previousState;
        MotionController& motionController; 
        volatile MotionController::SwitchState& switchState;

    public:
        FSM(MotionController& controller, volatile MotionController::SwitchState& switchState);
        void processCommand(const GCode& gcode);
        void update();
        void setState(State newState);
        State getState() const;
        const char* getStateName() const;
        const char* getStateName(State state) const;
};

#endif