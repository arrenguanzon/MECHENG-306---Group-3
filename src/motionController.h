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
        volatile bool sT_flag = false;
        volatile bool sB_flag = false;
        volatile bool sL_flag = false;
        volatile bool sR_flag = false;
        int homing_M2_Speed = 200;
        int homing_M1_Speed = 200;
        volatile bool homingComplete = false;

        
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