// #include <Arduino.h>

// #define motor1_pin 7
// #define enable1_pin 6
// #define enable2_pin 5
// #define motor2_pin 4


// #define switch_top 10
// #define switch_bottom 3
// #define switch_left 2
// #define switch_right A8

// #define ENCODER1_A 18
// #define ENCODER1_B 19
// #define ENCODER2_A 20
// #define ENCODER2_B 21


// int M2_speed = 75;
// int M1_speed = M2_speed * 1.3;


// typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
// typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
// volatile States state = IDLE;
// volatile SwitchState last_pressed = START;

// //function prototypes

// void TopISR();
// void BottomISR();
// void LeftISR();
// void RightISR();
// void Homing();
// void Delay(int delayTime);
// void HomingIdle();


// void setup()
// {
//     // Set up motor pins
//     pinMode(motor1_pin, OUTPUT);
//     pinMode(motor2_pin, OUTPUT);

//     Serial.begin(9600);
//     // Set up limit switch pins
//     pinMode(switch_top, INPUT_PULLUP);
//     pinMode(switch_bottom, INPUT_PULLUP);
//     pinMode(switch_left, INPUT_PULLUP);
//     pinMode(switch_right, INPUT_PULLUP);

//     attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
//     //Top switch
//     PCICR |= (1 << PCIE0); 
//     PCMSK0 |= (1 << PCINT4);
//     //Right switch
//     PCICR |= (1 << PCIE2);      
//     PCMSK2 |= (1 << PCINT16);
//     attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);

// }

// // new implements here

// enum HomingState {
//     MOVE_TO_LEFT,
//     BOTTOM_EDGE_CASE_WAIT,
//     MOVE_RIGHT,
//     WAIT_AFTER_RIGHT,
//     MOVE_TO_BOTTOM,
//     MOVE_UP,
//     HOMING_COMPLETE
// };

// HomingState homingState = MOVE_TO_LEFT;

// unsigned long motorStartTime = 0;
// unsigned long delayTime = 0;
// bool delayActive = false;

// void StartDelay(unsigned long ms) {
//     delayTime = ms;
//     motorStartTime = millis();
//     delayActive = true;
// }

// // Returns true once the delay (if any) has elapsed.
// bool DelayElapsed() {
//     if (!delayActive) return true;
//     if (millis() - motorStartTime >= delayTime) {
//         delayActive = false;
//         return true;
//     }
//     return false;
// }

// // Stops the motors
// void HomingIdle() {
//     analogWrite(enable1_pin, 0);
//     analogWrite(enable2_pin, 0);
// }

// // ISRs for the limit switches
// void BottomISR() { last_pressed = sB; }
// ISR(PCINT0_vect) { last_pressed = sT; }
// ISR(PCINT2_vect) { last_pressed = sR; }
// void LeftISR()   { last_pressed = sL; }

// void loop() {
//     Homing();
// }

// void Homing() {
//     if (!DelayElapsed()) return;

//     switch (homingState) {

//         case MOVE_TO_LEFT:
//             if (last_pressed == sB) {
//                 digitalWrite(motor1_pin, LOW);
//                 digitalWrite(motor2_pin, HIGH);
//                 analogWrite(enable1_pin, M1_speed);
//                 analogWrite(enable2_pin, M2_speed);
//                 StartDelay(1000);
//                 homingState = BOTTOM_EDGE_CASE_WAIT;
//             } else if (last_pressed == sL) {
//                 HomingIdle();
//                 StartDelay(1000);
//                 homingState = MOVE_RIGHT;
//             } else {
//                 digitalWrite(motor1_pin, LOW);
//                 digitalWrite(motor2_pin, LOW);
//                 analogWrite(enable1_pin, M1_speed);
//                 analogWrite(enable2_pin, M2_speed);
//             }
//             break;

//         case BOTTOM_EDGE_CASE_WAIT:
//             homingState = MOVE_TO_LEFT;
//             break;

//         case MOVE_RIGHT:
//             // delay elapsed, move right briefly
//             digitalWrite(motor1_pin, HIGH);
//             digitalWrite(motor2_pin, HIGH);
//             analogWrite(enable1_pin, M1_speed);
//             analogWrite(enable2_pin, M2_speed);
//             StartDelay(500);
//             homingState = WAIT_AFTER_RIGHT;
//             break;

//         case WAIT_AFTER_RIGHT:
//             // delay elapsed, idle briefly then move to bottom
//             HomingIdle();
//             StartDelay(1000);
//             homingState = MOVE_TO_BOTTOM;
//             break;

//         case MOVE_TO_BOTTOM:
//             if (last_pressed == sB) {
//                 HomingIdle();
//                 StartDelay(1000);
//                 homingState = MOVE_UP;
//             } else {
//                 digitalWrite(motor1_pin, HIGH);
//                 digitalWrite(motor2_pin, LOW);
//                 analogWrite(enable1_pin, M1_speed);
//                 analogWrite(enable2_pin, M2_speed);
//             }
//             break;

//         case MOVE_UP:
//             // delay elapsed, move up briefly
//             digitalWrite(motor1_pin, LOW);
//             digitalWrite(motor2_pin, HIGH);
//             analogWrite(enable1_pin, M1_speed);
//             analogWrite(enable2_pin, M2_speed);
//             StartDelay(500);
//             homingState = HOMING_COMPLETE;
//             break;

//         case HOMING_COMPLETE:
//         default:
//             // one-time settle happens on entry below, then this becomes a no-op
//             HomingIdle();
//             break;
//     }
// }



// //------------------------------------------

// // unsigned long motorStartTime;
// // int delayTime = 0;
// // bool delayActive = 0;
// // int alreadyDelayed = 0;

// // void Delay(int delayTime){
// //     if (millis() - motorStartTime >= delayTime) {
// //         delayActive = 0;
// //     }
// // }

// // void HomingIdle(){
// //     analogWrite(enable1_pin, 0);
// //     analogWrite(enable2_pin, 0);
// //     delayTime = 1000;
// //     motorStartTime = millis();
// //     delayActive = 1;
// // }

// // void BottomISR(){
// //     last_pressed = sB;
// // }

// // ISR(PCINT0_vect) {
// //    last_pressed = sT;
// // }

// // ISR(PCINT2_vect) {
// //     last_pressed = sR;
// // }

// // void LeftISR(){
// //     last_pressed = sL;
// // }



// // void loop(){
// //     if (millis() - motorStartTime >= delayTime & delayActive == 0) {
// //         Homing();
// //     }
// //     Delay(delayTime);
// // }


// // // void loop()
// // // {
// // //     digitalWrite(motor1_pin, LOW);
// // //     digitalWrite(motor2_pin, LOW);
// // //     analogWrite(enable1_pin, 100);
// // //     analogWrite(enable2_pin, 100);
// // // }


// // void Homing(){
// //     while(last_pressed != sL){
// //         if(last_pressed == sB){ // edge case for starting at bottom switch
// //             digitalWrite(motor1_pin, LOW);
// //             digitalWrite(motor2_pin, HIGH);
// //             analogWrite(enable1_pin, M1_speed);
// //             analogWrite(enable2_pin, M2_speed);
// //             delayTime = 1000;
// //             motorStartTime = millis();
// //             delayActive = 1; 
// //             return;
// //         }
// //         // move motors to the left until the left switch is pressed
// //         digitalWrite(motor1_pin, LOW);
// //         digitalWrite(motor2_pin, LOW);
// //         analogWrite(enable1_pin, M1_speed);
// //         analogWrite(enable2_pin, M2_speed);
// //     }
    
// //     if(alreadyDelayed == 0){
// //         delayTime = 1000;
// //         motorStartTime = millis();
// //         delayActive = 1; 
// //         alreadyDelayed = 1;
// //         return;
// //     }

// //     // move motors to the right
// //     digitalWrite(motor1_pin, HIGH);
// //     digitalWrite(motor2_pin, HIGH);
// //     analogWrite(enable1_pin, M1_speed);
// //     analogWrite(enable2_pin, M2_speed);

// //     if(alreadyDelayed == 1){
// //         delayTime = 500;
// //         motorStartTime = millis();
// //         delayActive = 1; 
// //         alreadyDelayed = 2;
// //         return;
// //     }

// //     if(alreadyDelayed == 2){
// //         alreadyDelayed = 3;
// //         HomingIdle();
// //         return;
// //     }

// //     while(last_pressed != sB){ // move motors down until the bottom switch is pressed
// //         Serial.println(last_pressed);
// //         digitalWrite(motor1_pin, HIGH);
// //         digitalWrite(motor2_pin, LOW);
// //         analogWrite(enable1_pin, M1_speed);
// //         analogWrite(enable2_pin, M2_speed);
// //     }

// //     if(alreadyDelayed == 3){
// //         HomingIdle();
// //         delayTime = 0;
// //         delayActive = 0;
// //     }
// //     //switchcase
// //     while(1);
// // }