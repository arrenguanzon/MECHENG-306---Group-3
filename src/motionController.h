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
        int targetMotor1; 
        int targetMotor2;

        // Reference to absolute position variables
        float& absoluteX;
        float& absoluteY;
        const volatile SwitchState& last_pressed;

        int motor1Error;
        int motor2Error;

        bool moving_completed;

        // Homing state machine and variables
        enum HomingState {
            MOVE_TO_LEFT,
            BOTTOM_EDGE_CASE_WAIT,
            TOP_EDGE_CASE_WAIT,
            MOVE_RIGHT,
            WAIT_AFTER_RIGHT,
            MOVE_TO_BOTTOM,
            MOVE_UP,
            HOMING_COMPLETE
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

        MotionController(Encoder& encoder, float& absoluteX, float& absoluteY, const volatile SwitchState& last_pressed);
        void setTarget(float x, float y, float speed);
        void update();
        bool isCompleted() const;
        void calculateMotorTargets();

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