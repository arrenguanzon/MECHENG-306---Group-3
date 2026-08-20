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

// #define DEBOUNCE_MS 50


// // int M2_speed = 75;
// // int M1_speed = 75;//M2_speed * 1.3;


// typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
// typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
// volatile States state = IDLE;
// //volatile SwitchState last_pressed = START;

// //function prototypes

// void TopISR();
// void BottomISR();
// void LeftISR();
// void RightISR();
// void Homing();
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

// // Stops the motors
// void HomingIdle() {
//     analogWrite(enable1_pin, 0);
//     analogWrite(enable2_pin, 0);
// }

// volatile unsigned long last_sT_time = 0;
// volatile unsigned long last_sB_time = 0;
// volatile unsigned long last_sL_time = 0;
// volatile unsigned long last_sR_time = 0;

// volatile SwitchState last_pressed = START;
// HomingState homingState = MOVE_TO_LEFT;

// // ISRs for the limit switches
// void BottomISR() {
//     unsigned long now = millis();
//     if (now - last_sB_time >= DEBOUNCE_MS) {
//         last_pressed = sB;
//         last_sB_time = now;
//     }
// }

// ISR(PCINT0_vect) {
//     unsigned long now = millis();
//     if (digitalRead(switch_top) == LOW && (now - last_sT_time >= DEBOUNCE_MS)) {
//         last_pressed = sT;
//         last_sT_time = now;
//     }
// }

// ISR(PCINT2_vect) {
//     unsigned long now = millis();
//     if (digitalRead(switch_right) == LOW && (now - last_sR_time >= DEBOUNCE_MS)) {
//         last_pressed = sR;
//         last_sR_time = now;
//     }
// }

// void LeftISR() {
//     unsigned long now = millis();
//     if (now - last_sL_time >= DEBOUNCE_MS) {
//         last_pressed = sL;
//         last_sL_time = now;
//     }
// }

// // void loop() {
// //     Homing();
// // }

// int M2_speed = 100;
// int M1_speed = 130;

// void loop() {
//     digitalWrite(motor1_pin, LOW);
//     digitalWrite(motor2_pin, LOW);
//     analogWrite(enable1_pin, 130);
//     analogWrite(enable2_pin, 100);
// }

// void Homing() {

//     switch (homingState) {

//         case MOVE_TO_LEFT:
//             if (last_pressed == sB) {
//                 digitalWrite(motor1_pin, LOW);
//                 digitalWrite(motor2_pin, HIGH);
//                 analogWrite(enable1_pin, M1_speed);
//                 analogWrite(enable2_pin, M2_speed);
//                 delay(1000);
//                 //StartDelay(1000);
//                 homingState = BOTTOM_EDGE_CASE_WAIT;
//             } else if (last_pressed == sL) {
//                 HomingIdle();
//                 homingState = MOVE_RIGHT;
//             } else {
//                 digitalWrite(motor1_pin, LOW);
//                 digitalWrite(motor2_pin, LOW);
//                 analogWrite(enable1_pin, M1_speed);
//                 analogWrite(enable2_pin, M2_speed);
//             }
//             break;

//         case BOTTOM_EDGE_CASE_WAIT:
//             last_pressed = START;
//             homingState = MOVE_TO_LEFT;
//             break;

//         case MOVE_RIGHT:
//             // delay elapsed, move right briefly
//             digitalWrite(motor1_pin, HIGH);
//             digitalWrite(motor2_pin, HIGH);
//             analogWrite(enable1_pin, M1_speed);
//             analogWrite(enable2_pin, M2_speed);
//             delay(500);
//             //StartDelay(500);
//             homingState = WAIT_AFTER_RIGHT;
//             break;

//         case WAIT_AFTER_RIGHT:
//             // delay elapsed, idle briefly then move to bottom
//             HomingIdle();
//             //StartDelay(1000);
//             homingState = MOVE_TO_BOTTOM;
//             break;

//         case MOVE_TO_BOTTOM:
//             if (last_pressed == sB) {
//                 HomingIdle();
//                 //StartDelay(1000);
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
//             delay(500);
//             //StartDelay(500);
//             homingState = HOMING_COMPLETE;
//             break;

//         case HOMING_COMPLETE:
//             HomingIdle();
//             break;
//     }
// }