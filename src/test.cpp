// #include <Arduino.h>
// #include "encoder.h"


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
// void ENCODER1AISR();
// void ENCODER1BISR();
// void ENCODER2AISR();
// void ENCODER2BISR();


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

//     pinMode(ENCODER1_A, INPUT_PULLUP);
//     pinMode(ENCODER1_B, INPUT_PULLUP);
//     pinMode(ENCODER2_A, INPUT_PULLUP);
//     pinMode(ENCODER2_B, INPUT_PULLUP);

//     attachInterrupt(digitalPinToInterrupt(ENCODER1_A), ENCODER1AISR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER1_B), ENCODER1BISR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER2_A), ENCODER2AISR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER2_B), ENCODER2BISR, CHANGE);

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

// volatile bool sT_flag = false;
// volatile bool sB_flag = false;
// volatile bool sL_flag = false;
// volatile bool sR_flag = false;

// Encoder encoder;

// volatile SwitchState last_pressed = START;
// HomingState homingState = MOVE_TO_LEFT;

// // ISRs for the limit switches
// void BottomISR() {
//     unsigned long now = millis();
//     if (now - last_sB_time >= DEBOUNCE_MS) {
//         sB_flag = true;
//         last_sB_time = now;
//     }
// }

// ISR(PCINT0_vect) {
//     unsigned long now = millis();
//     if (digitalRead(switch_top) == LOW && (now - last_sT_time >= DEBOUNCE_MS)) {
//          sT_flag = true;
//         last_sT_time = now;
//     }
// }

// ISR(PCINT2_vect) {
//     unsigned long now = millis();
//     if (digitalRead(switch_right) == LOW && (now - last_sR_time >= DEBOUNCE_MS)) {
//          sR_flag = true;
//         last_sR_time = now;
//     }
// }

// void LeftISR() {
//     unsigned long now = millis();
//     if (now - last_sL_time >= DEBOUNCE_MS) {
//          sL_flag = true;
//         last_sL_time = now;
//     }
// }
// void ENCODER1AISR() {
//     bool A = digitalRead(ENCODER1_A);
//     bool B = digitalRead(ENCODER1_B);

//     if (A == B) {
//         encoder.incrementMotor1Count();
//     } else {
//         encoder.decrementMotor1Count();
//     }
// }

// void ENCODER1BISR() {
//     bool A = digitalRead(ENCODER1_A);
//     bool B = digitalRead(ENCODER1_B);

//     if (A != B) {
//         encoder.incrementMotor1Count();
//     } else {
//         encoder.decrementMotor1Count();
//     }
// }

// void ENCODER2AISR() {
//     bool A = digitalRead(ENCODER2_A);
//     bool B = digitalRead(ENCODER2_B);

//     if (A == B) {
//         encoder.decrementMotor2Count();
//     } else {
//         encoder.incrementMotor2Count();
//     }
// }

// void ENCODER2BISR() {
//     bool A = digitalRead(ENCODER2_A);
//     bool B = digitalRead(ENCODER2_B);

//     if (A != B) {
//         encoder.decrementMotor2Count();
//     } else {
//         encoder.incrementMotor2Count();
//     }
// }

// // void loop() {
// //     Homing();
// // }



// int M2_speed = 100;
// int M1_speed = 130;

// long current1 = encoder.getMotor1Count();
// long current2 = encoder.getMotor2Count();

// void loop() {

//     static unsigned long lastEncoderPrint = 0;

// if (millis() - lastEncoderPrint >= 250) {
//     lastEncoderPrint = millis();

//     Serial.print("Encoder 1: ");
//     Serial.print(encoder.getMotor1Count());

//     Serial.print(" | Encoder 2: ");
//     Serial.println(encoder.getMotor2Count());
// }


//     digitalWrite(motor1_pin, LOW);
//     digitalWrite(motor2_pin, LOW);
//     analogWrite(enable1_pin, 100);
//     analogWrite(enable2_pin, 0);

//     if(sB_flag) {
//         Serial.println("bottom");
//         sB_flag = false;
//     }

//     if(sT_flag) {
//         Serial.println("top");
//         sT_flag = false;
//     }

//      if(sL_flag) {
//         Serial.println("tleft");
//         sL_flag = false;
//     }
//      if(sR_flag) {
//         Serial.println("right");
//         sR_flag = false;
//     }
// }


