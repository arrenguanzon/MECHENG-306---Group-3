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
                switchState = MotionController::START;
                motionController.StartHoming();
                setState(HOMING);
            }
            else if (gcode.getCommand() == GCode::G1) {
                Serial.print("X target: ");
                Serial.println(gcode.getXTarget());

                Serial.print("Y target: ");
                Serial.println(gcode.getYTarget());

                Serial.print("Speed target: ");
                Serial.println(gcode.getSpeedTarget());
                motionController.setTarget(
                    gcode.getXTarget(),
                    gcode.getYTarget(),
                    gcode.getSpeedTarget()
                );
                setState(MOVING);
            }
            break;

        case HOMING:
            // Don't accept another homing command
            // while currently homing.
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
            motionController.HomingFunction();
            if (motionController.isHomingComplete()) {
                setState(IDLE);
            }

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


