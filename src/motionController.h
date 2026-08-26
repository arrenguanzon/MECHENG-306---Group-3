#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <Arduino.h>
#include "encoder.h"
#include "gcode.h"

class MotionController {
    public:
        enum SwitchState {sT, sB, sL, sR, START};
    private:
        Encoder& encoder;

        float targetX;
        float targetY;
        float speed;

        // Target Motor Encoder Counts
        long targetMotor1; 
        long targetMotor2;

        // Reference to absolute position variables
        float& absoluteX;
        float& absoluteY;
        volatile SwitchState& last_pressed;

        // Velocity profile: snapshot of encoder counts at the start of the current move
        long startMotor1;
        long startMotor2;

        // Velocity profile: total distance (in counts) each motor must travel this move
        float dist1Total;
        float dist2Total;

        // Velocity profile: true Cartesian path length for this move, in counts
        float pathLengthCounts;

        bool moving_completed;

        // Homing state machine and variables
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
        HomingState homingState = MOVE_TO_LEFT;
        volatile bool homingComplete = false;

        // PI control variables
        // Track previous integrals
        float prevIntegralMotor1;
        float prevIntegralMotor2;

        // Integral terms for PID control
        float integralMotor1;
        float integralMotor2;

        
    public:

        MotionController(Encoder& encoder, float& absoluteX, float& absoluteY, volatile SwitchState& last_pressed);
        void setTarget(float x, float y, float speed);
        void update();
        bool isCompleted() const;
        void calculateMotorTargets();
        void updateAbsolutePosition();
        float calculateVelocityCeiling(float distanceTraveled) const;

        // State motion controls
        void Idle();
        // Homing state functions
        void HomingIdle();
        void HomingFunction();
        bool isHomingComplete() const;
        void StartHoming();
        void HomingFlagSetter();

};

#endif // MOTIONCONTROLLER_H