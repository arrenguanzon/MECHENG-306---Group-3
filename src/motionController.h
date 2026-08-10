#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <Arduino.h>
#include "encoder.h"

class MotionController {
    private:
        Encoder& encoder;

        float currentX;
        float currentY;

        float targetX;
        float targetY;

        float speed;

        bool completed;
        void updatePosition();
        
    public:
        MotionController(Encoder& encoder);
        void setTarget(float x, float y, float speed);
        void update();
        bool isCompleted() const;
        float getCurrentX() const;
        float getCurrentY() const;
};

#endif // MOTIONCONTROLLER_H