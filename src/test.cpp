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


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;

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
