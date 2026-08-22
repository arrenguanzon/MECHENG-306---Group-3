#ifndef HOMING_H
#define HOMING_H

#include <Arduino.h>
#include <math.h>
#include <string.h>

class Homing {

    public:
 
    static Homing* getInstance() {
        if (instance == nullptr) { 
            instance = new Homing();
        }
        return instance;
    }

    void startHoming() {
        homingState = MOVE_TO_LEFT;
        homingComplete = false;
        sT_flag = false;
        sB_flag = false;
        sL_flag = false;
        sR_flag = false;
    }

    void homingFunction();

    volatile unsigned long last_sT_time = 0;
    volatile unsigned long last_sB_time = 0;
    volatile unsigned long last_sL_time = 0;
    volatile unsigned long last_sR_time = 0;

    volatile bool sT_flag = false;
    volatile bool sB_flag = false;
    volatile bool sL_flag = false;
    volatile bool sR_flag = false;

    volatile bool homingComplete = false;

    private:
    Homing(); // Private constructor
    
   //Changed instance declaration to a pointer
    static Homing* instance; 

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

    void homingIdle();

    int M2_speed = 225;
    int M1_speed = 255;
};

#endif
