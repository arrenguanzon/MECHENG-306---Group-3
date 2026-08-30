#include "fsm.h"

// Initialise FSM with references to motion controller and switch state
FSM::FSM(MotionController& controller, volatile MotionController::SwitchState& switchState) : motionController(controller), switchState(switchState)
{
    state = IDLE;
    previousState = IDLE;
}

// Process incoming G-code command based on current state
void FSM::processCommand(const GCode& gcode)
{
    // Invalid commands are ignored
    if (!gcode.isValid()) {
        return;
    }

    switch (state) {
        case IDLE:
            if (gcode.getCommand() == GCode::G28) {
                // Homing command received begin homing sequence
                switchState = MotionController::START;
                motionController.StartHoming();
                setState(HOMING);
            }
            else if (gcode.getCommand() == GCode::G1) {
                // Movement command received extract target and speed
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
            // Ignore new commands while homing
            break;

        case MOVING:
            // Ignore new commands while moving
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
    // Print state transitions for debugging
    if (state != previousState) {
        Serial.print("FSM State: ");
        Serial.print(getStateName(previousState));
        Serial.print(" -> ");
        Serial.println(getStateName(state));

        previousState = state;
    }

    // Execute state specific logic
    switch (state)
    {
        case IDLE:
            // Nothing to do - waiting for commands
            break;

        case HOMING:
            // Execute homing state machine
            motionController.HomingFunction();
            if (motionController.isHomingComplete()) {
                setState(IDLE);
            }

            break;

        case MOVING:
            // Execute movement update
            motionController.update();
            if (motionController.isCompleted()){
                setState(IDLE);
            }
            break;

        case FAULT:
            // Stop all motors in fault state
            motionController.Idle();
            break;
    }
}

// Get current state
FSM::State FSM::getState() const{
    return state;
}

// Change to new state
void FSM::setState(State newState) {
    // Update state variable
    state = newState;
}

// Get string name of current state
const char* FSM::getStateName() const {
    return getStateName(state);
}

// Get string name of specified state (for debug output)
const char* FSM::getStateName(State state) const {
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