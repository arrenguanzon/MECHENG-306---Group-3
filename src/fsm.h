#ifndef FSM_H
#define FSM_H

#include "gcode.h"
#include "motionController.h"

class FSM {
    public:
        enum State
        {
            IDLE, // Plotter waiting for commands
            HOMING, // Plotter performing homing sequence
            MOVING, // Plotter executing G1 movement command
            FAULT // Plotter in fault state due to limit switch activation
        };

    private:
        State state; // Current state
        State previousState;  // Previous state for detecting transitions
        MotionController& motionController; // Reference to motion controller
        volatile MotionController::SwitchState& switchState; // Reference to switch state

    public:
        // Initialise FSM with motion controller and switch state reference
        FSM(MotionController& controller, volatile MotionController::SwitchState& switchState);
        
        // Process a G-code command based on current state
        void processCommand(const GCode& gcode);
        
        // Update FSM 
        void update();
        
        // Change to a new state
        void setState(State newState);
        
        // Get current state
        State getState() const;
        
        // Get string name of current state
        const char* getStateName() const;
        
        // Get string name of specified state
        const char* getStateName(State state) const;
};

#endif