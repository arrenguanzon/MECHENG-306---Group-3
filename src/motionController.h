#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <Arduino.h>
#include "encoder.h"

class MotionController {
    private:
        Encoder& encoder;

        float targetX;
        float targetY;
        float speed; // target speed from g-code parsing  

        // Target Motor Encoder Counts
        long targetMotor1; 
        long targetMotor2;

        // Trapezoidal velocity profile parameters
        unsigned long moveStartTime; // tracks when motion started
        float startSpeed;            // minimum starting speed
        float accelRate;             // acceleration rate in PWM units per millisecond

        // Reference to absolute position variables
        float& absoluteX;
        float& absoluteY;

        // Track previous integrals
        float prevIntegralMotor1;
        float prevIntegralMotor2;

        // Integral terms for PID control
        float integralMotor1;
        float integralMotor2;

        bool completed;
        
    public:
        enum SwitchState {sT, sB, sL, sR, START};

        MotionController(Encoder& encoder, float& absoluteX, float& absoluteY);
        void setTarget(float x, float y, float speed);
        void update();
        bool isCompleted() const;
        void calculateMotorTargets();

        // State motion controls
        void Idle();
        void Homing(const volatile SwitchState& switchState);
};

#endif // MOTIONCONTROLLER_H