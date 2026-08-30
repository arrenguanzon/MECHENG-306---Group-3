#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <Arduino.h>
#include "encoder.h"
#include "gcode.h"

class MotionController {
    public:
        // Limit switch states
        enum SwitchState {
            sT,    // Top limit switch pressed
            sB,    // Bottom limit switch pressed
            sL,    // Left limit switch pressed
            sR,    // Right limit switch pressed
            START  // Initial or no switch pressed state
        };
    private:
        Encoder& encoder; // Reference to encoder for position tracking

        float targetX; // Target X displacement (mm)
        float targetY; // Target Y displacement (mm)
        float speed;  // Commanded speed (PWM value)

        // Target Motor Encoder Counts
        long targetMotor1; // Target encoder count for motor 1
        long targetMotor2; // Target encoder count for motor 2

        float& absoluteX; // Reference to absolute X position
        float& absoluteY; // Reference to absolute Y position
        volatile SwitchState& last_pressed;  // Reference to last pressed limit switch
  
        // Velocity Profile 
        long startMotor1;
        long startMotor2;

        // Total distance (in counts) each motor must travel for current move
        float dist1Total;
        float dist2Total;

        // True cartesian path length for current move, in encoder counts
        float pathLengthCounts;

        bool moving_completed;    // Flag indicating if movement is complete

        // Homing 
        enum HomingState {
            MOVE_TO_LEFT,
            BOTTOM_EDGE_CASE_WAIT,
            MOVE_RIGHT,
            WAIT_AFTER_RIGHT,
            MOVE_TO_BOTTOM,
            MOVE_UP,
            HOMING_COMPLETE,
            WAIT_AFTER_UP,
            MOVE_TO_LEFT2,
            MOVE_RIGHT2,
            WAIT_AFTER_RIGHT2,
            MOVE_TO_BOTTOM2,
            MOVE_UP2,
            WAIT_AFTER_UP2
        };
        HomingState homingState = MOVE_TO_LEFT;  // Current homing state
        volatile bool homingComplete = false;   // Homing finished
        volatile bool homingRunning = false;    // Homing in progress

        // PI Control 
        // Previous integral terms (for anti-windup clamping)
        float prevIntegralMotor1;
        float prevIntegralMotor2;

        // Current integral terms for PI control
        float integralMotor1;
        float integralMotor2;

        // Low-Speed Pulse Mode for low-speed pulse generation when ratio between M1 and M2 
        // isn't reachable due to required M2 being lower than minimum speed
        float dutyAccumulator1;
        float dutyAccumulator2;
        
        // Flag: pulse currently active (motor running at min speed)
        bool pulseActive1;
        bool pulseActive2;
        
        // Timestamp when current pulse started
        unsigned long pulseStartMillis1;
        unsigned long pulseStartMillis2;

        // Debug ticks to prevent overload due to debug prints
        static const unsigned long DEBUG_PRINT_INTERVAL_MS = 200;
        unsigned long lastDebugPrint = 0;
        bool debugTick();
        
    public:
        // Initialise motion controller with encoder and position references
        MotionController(Encoder& encoder, float& absoluteX, float& absoluteY, volatile SwitchState& last_pressed);
        
        // Set movement target and speed (x, y in mm, speed as PWM value)
        void setTarget(float x, float y, float speed);
        
        // Execute one control loop iteration
        void update();
        
        // Check if current movement is complete
        bool isCompleted() const;
        
        // Calculate target motor encoder counts from XY target position
        void calculateMotorTargets();
        
        // Update absolute position with completed movement
        void updateAbsolutePosition();
        
        // Calculate maximum allowed speed at given distance (velocity profile)
        // Returns maximum allowed speed based on acceleration/deceleration curve
        float calculateVelocityCeiling(float distanceTraveled) const;

        // Stop motors immediately
        void Idle();
        
        // Stop motors during homing 
        void HomingIdle();
        
        // Executes homing
        void HomingFunction();
        
        // Check if homing is complete
        bool isHomingComplete() const;
        
        // Initialise homing 
        void StartHoming();

};

#endif // MOTIONCONTROLLER_H