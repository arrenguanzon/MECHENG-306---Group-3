#include "fsm.h"

FSM::FSM(MotionController& controller, volatile MotionController::SwitchState& switchState) : motionController(controller), switchState(switchState)
{
    state = IDLE;
}

void FSM::processCommand(const GCode& gcode)
{
    // Invalid commands are ignored for now
    if (!gcode.isValid()) {
        return;
    }

    switch (state) {
        case IDLE:
            if (gcode.getCommand() == GCode::G28) {
                setState(HOMING);
            }
            else if (gcode.getCommand() == GCode::G1) {
                motionController.setTarget(
                    gcode.getXTarget(),
                    gcode.getYTarget(),
                    gcode.getSpeedTarget()
                );
                setState(MOVING);
            }
            break;

        case HOMING:
            // We'll implement this later
            break;

        case MOVING:
            // Don't accept another movement command
            // while currently moving.
            break;

        case FAULT:
            if (gcode.getCommand() == GCode::M999){
                setState(IDLE);
            }
            break;
    }
}

void FSM::update()
{
    switch (state)
    {
        case IDLE:
            // Nothing to continuously do
            break;

        case HOMING:
             motionController.Homing(switchState);
            // Homing has finished once Homing() returns
            setState(IDLE);

            break;

        case MOVING:
            motionController.update();
            if (motionController.isCompleted()){
                setState(IDLE);
            }
            break;

        case FAULT:
            motionController.Idle();
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


