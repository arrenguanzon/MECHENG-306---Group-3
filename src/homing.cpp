// #include "homing.h"
// #include "fsm.h"

// Homing* Homing::instance = nullptr;

// // #define motor1_pin 7
// // #define enable1_pin 6
// // #define enable2_pin 5
// // #define motor2_pin 4


// // #define switch_top 10
// // #define switch_bottom 3
// // #define switch_left 2
// // #define switch_right A8


// // Stops the motors


// Homing::Homing(){

// }



// void Homing::homingIdle() {
//     analogWrite(6, 0); 
//     analogWrite(5, 0);
// }
// void Homing::homingFunction(
//     MotionController& motionController,
//     volatile MotionController::SwitchState& switchState
// ){

//     switch (homingState) {

//         case MOVE_TO_LEFT:
//             if (sB_flag) {
//             //if (last_pressed == sB) {
//                 sB_flag = false;
//                 switchState = MotionController::START; //so that it dosen't fault because motion controll treats lm as a fault
//                 motionController.setTarget(0.0f, 5.0f, 50.0f);
//                 homingState = BOTTOM_EDGE_CASE_WAIT;
//             } else if (sT_flag) {
//                 sT_flag = false;
//                 digitalWrite(7, HIGH);
//                 digitalWrite(4, LOW);
//                 analogWrite(6, M1_speed);
//                 analogWrite(5, M2_speed);
//                 switchState = MotionController::START;
//                 motionController.setTarget(0.0f, -5.0f, 50.0f);
//                 homingState = TOP_EDGE_CASE_WAIT;
//             }else if (sL_flag) {
//             //} else if (last_pressed == sL) {
//                 sL_flag = false;
//                 homingIdle();
//                 homingState = MOVE_RIGHT;
//             } else {
//                 digitalWrite(7, LOW);
//                 digitalWrite(4, LOW);
//                 analogWrite(6, M1_speed);
//                 analogWrite(5, M2_speed);
//             }
//             break;

//         case BOTTOM_EDGE_CASE_WAIT:
//             motionController.update(switchState);
//             if (motionController.isCompleted()) {
//                 homingState = MOVE_TO_LEFT;
//             }
//             break;
        
//         case TOP_EDGE_CASE_WAIT:
//             //last_pressed = START;
//             homingState = MOVE_TO_LEFT;
//             break;

//         case MOVE_RIGHT:
//             digitalWrite(7, HIGH);
//             digitalWrite(4, HIGH);
//             analogWrite(6, M1_speed);
//             analogWrite(5, M2_speed);
//             switchState = MotionController::START; //so that it dosen't fault because motion controll treats lm as a fault
//             motionController.setTarget(5.0f, 0.0f, 50.0f);
//             homingState = WAIT_AFTER_RIGHT;
//             break;

//         case WAIT_AFTER_RIGHT:
//             homingIdle();
//             homingState = MOVE_TO_BOTTOM;
//             break;

//         case MOVE_TO_BOTTOM:
//             if (sB_flag) {
//             //if (last_pressed == sB) {
//                 sB_flag = false;
//                 homingIdle();
//                 homingState = MOVE_UP;
//             } else {
//                 digitalWrite(7, HIGH);
//                 digitalWrite(4, LOW);
//                 analogWrite(6, M1_speed);
//                 analogWrite(5, M2_speed);
//             }
//             break;

//         case MOVE_UP:
//             digitalWrite(7, LOW);
//             digitalWrite(4, HIGH);
//             analogWrite(6, M1_speed);
//             analogWrite(5, M2_speed);
//             switchState = MotionController::START; //so that it dosen't fault because motion controll treats lm as a fault
//             motionController.setTarget(0.0f, 5.0f, 50.0f);
//             homingState = HOMING_COMPLETE;
//             break;

//         case HOMING_COMPLETE:
//             homingIdle();
//             homingComplete = true;
            
//             break;
//     }
    

// }