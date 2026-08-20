#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <Arduino.h>
#include "encoder.h"
#include "gcode.h"

class MotionController {
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

        int motor1Error;
        int motor2Error;

        bool completed;
        
    public:
        enum SwitchState {sT, sB, sL, sR, START};

        MotionController(Encoder& encoder, float& absoluteX, float& absoluteY);
        void setTarget(float x, float y, float speed);
        void update(const volatile SwitchState& switchState);
        bool isCompleted() const;
        void calculateMotorTargets();

        // State motion controls
        void Idle();
        void Homing(const volatile SwitchState& switchState);
        //bool Fault(const volatile SwitchState& switchState);
};

#endif // MOTIONCONTROLLER_H