#include "fsm.h"

FSM::FSM()
{
    state = IDLE;
}

void FSM::update(const GCode& gcode)
{
    if (!gcode.isValid()) {
        return;
    }
    switch (state)
    {
        case IDLE:
            if(gcode.getCommand() == GCode::G28) {
                setState(HOMING);
            } else if (gcode.getCommand() == GCode::G1) {
                setState(MOVING);
            }
            break;

        case HOMING:

            break;

        case MOVING:

            break;

        case FAULT:
            if (gcode.getCommand() == GCode::M999) {
                setState(IDLE);
            }
            break;
    }
}


FSM::State FSM::getState() const{
    return state;
}

void FSM::setState(State newState) {
    state = newState;
}

const char* FSM::getStateName() const {
    switch (state) {
        case IDLE:
            return "IDLE";
        case HOMING:
            return "HOMING";
        case MOVING:
            return "MOVING";
        case FAULT:
            return "FAULT";
        default:
            return "UNKNOWN";
    }
}


