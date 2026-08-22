#include "homing.h"
#include "fsm.h"

Homing* Homing::instance = nullptr;

// #define motor1_pin 7
// #define enable1_pin 6
// #define enable2_pin 5
// #define motor2_pin 4


// #define switch_top 10
// #define switch_bottom 3
// #define switch_left 2
// #define switch_right A8


// Stops the motors


Homing::Homing(){

}

void Homing::homingIdle() {
    analogWrite(6, 0); 
    analogWrite(5, 0);
}

void Homing::homingFunction(){

    switch (homingState) {

        case MOVE_TO_LEFT:
            if (sB_flag) {
            //if (last_pressed == sB) {
                sB_flag = false;
                digitalWrite(7, LOW);
                digitalWrite(4, HIGH);
                analogWrite(6, M1_speed);
                analogWrite(5, M2_speed);
                delay(1000);
                homingState = BOTTOM_EDGE_CASE_WAIT;
            } else if (sT_flag) {
                sT_flag = false;
                digitalWrite(7, HIGH);
                digitalWrite(4, LOW);
                analogWrite(6, M1_speed);
                analogWrite(5, M2_speed);
                delay(1000);
                homingState = TOP_EDGE_CASE_WAIT;
            }else if (sL_flag) {
            //} else if (last_pressed == sL) {
                sL_flag = false;
                homingIdle();
                homingState = MOVE_RIGHT;
            } else {
                digitalWrite(7, LOW);
                digitalWrite(4, LOW);
                analogWrite(6, M1_speed);
                analogWrite(5, M2_speed);
            }
            break;

        case BOTTOM_EDGE_CASE_WAIT:
            //last_pressed = START;
            homingState = MOVE_TO_LEFT;
            break;
        
        case TOP_EDGE_CASE_WAIT:
            //last_pressed = START;
            homingState = MOVE_TO_LEFT;
            break;

        case MOVE_RIGHT:
            // delay elapsed, move right briefly
            digitalWrite(7, HIGH);
            digitalWrite(4, HIGH);
            analogWrite(6, M1_speed);
            analogWrite(5, M2_speed);
            delay(500);
            homingState = WAIT_AFTER_RIGHT;
            break;

        case WAIT_AFTER_RIGHT:
            // delay elapsed, idle briefly then move to bottom
            homingIdle();
            homingState = MOVE_TO_BOTTOM;
            break;

        case MOVE_TO_BOTTOM:
            if (sB_flag) {
            //if (last_pressed == sB) {
                sB_flag = false;
                homingIdle();
                homingState = MOVE_UP;
            } else {
                digitalWrite(7, HIGH);
                digitalWrite(4, LOW);
                analogWrite(6, M1_speed);
                analogWrite(5, M2_speed);
            }
            break;

        case MOVE_UP:
            // delay elapsed, move up briefly
            digitalWrite(7, LOW);
            digitalWrite(4, HIGH);
            analogWrite(6, M1_speed);
            analogWrite(5, M2_speed);
            delay(500);
            homingState = HOMING_COMPLETE;
            break;

        case HOMING_COMPLETE:
            homingIdle();
            homingComplete = true;
            
            break;
    }
    

}