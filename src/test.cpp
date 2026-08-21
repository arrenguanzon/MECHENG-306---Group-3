#include <Arduino.h>


#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4


#define switch_top 10
#define switch_bottom 3
#define switch_left 2
#define switch_right A8

#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21

#define DEBOUNCE_MS 25


// int M2_speed = 75;
// int M1_speed = 75;//M2_speed * 1.15;


//typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
//volatile SwitchState last_pressed = START;

//function prototypes

void TopISR();
void BottomISR();
void LeftISR();
void RightISR();
void Homing();
void HomingIdle();

void setup()
{
    // Set up motor pins
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);

    Serial.begin(9600);
    // Set up limit switch pins
    pinMode(switch_top, INPUT_PULLUP);
    pinMode(switch_bottom, INPUT_PULLUP);
    pinMode(switch_left, INPUT_PULLUP);
    pinMode(switch_right, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
    //Top switch
    PCICR |= (1 << PCIE0); 
    PCMSK0 |= (1 << PCINT4);
    //Right switch
    PCICR |= (1 << PCIE2);      
    PCMSK2 |= (1 << PCINT16);
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);

}

// new implements here

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

// Stops the motors
void HomingIdle() {
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

volatile unsigned long last_sT_time = 0;
volatile unsigned long last_sB_time = 0;
volatile unsigned long last_sL_time = 0;
volatile unsigned long last_sR_time = 0;

// Independent flags, one per switch, instead of a single shared "last_pressed"
volatile bool sT_flag = false;
volatile bool sB_flag = false;
volatile bool sL_flag = false;
volatile bool sR_flag = false;

HomingState homingState = MOVE_TO_LEFT;

// ISRs for the limit switches
void BottomISR() {
    //Serial.println("BOTTOM flag set");
    unsigned long now = millis();
    if (now - last_sB_time >= DEBOUNCE_MS) {
        sB_flag = true;
        last_sB_time = now;
    }
}

ISR(PCINT0_vect) {
    //Serial.println("TOP flag set");
    unsigned long now = millis();
    if (digitalRead(switch_top) == LOW && (now - last_sT_time >= DEBOUNCE_MS)) {
        sT_flag = true;
        last_sT_time = now;
    }
}

ISR(PCINT2_vect) {
   // Serial.println("RIGHT flag set");
    unsigned long now = millis();
    if ((digitalRead(switch_right) == LOW && (now - last_sR_time >= DEBOUNCE_MS)) ) { {
        sR_flag = true;
        last_sR_time = now;
    }
}
}

void LeftISR() {
    //Serial.println("LEFT flag set");
    unsigned long now = millis();
    if (now - last_sL_time >= DEBOUNCE_MS) {
        sL_flag = true;
        last_sL_time = now;
    }
}

// void loop() {
//     Homing();
// }

int M2_speed = 225;
int M1_speed = 255;

// void loop() {
//     digitalWrite(motor1_pin, LOW);
//     digitalWrite(motor2_pin, HIGH);
//     analogWrite(enable1_pin, 115);
//     analogWrite(enable2_pin, 100);
// }

void Homing() {

    switch (homingState) {

        case MOVE_TO_LEFT:
            if (sB_flag) {
            //if (last_pressed == sB) {
                sB_flag = false;
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, HIGH);
                analogWrite(enable1_pin, M1_speed);
                analogWrite(enable2_pin, M2_speed);
                delay(1000);
                homingState = BOTTOM_EDGE_CASE_WAIT;
            } else if (sT_flag) {
                sT_flag = false;
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, M1_speed);
                analogWrite(enable2_pin, M2_speed);
                delay(1000);
                homingState = TOP_EDGE_CASE_WAIT;
            }else if (sL_flag) {
            //} else if (last_pressed == sL) {
                sL_flag = false;
                HomingIdle();
                homingState = MOVE_RIGHT;
            } else {
                digitalWrite(motor1_pin, LOW);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, M1_speed);
                analogWrite(enable2_pin, M2_speed);
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
            digitalWrite(motor1_pin, HIGH);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, M1_speed);
            analogWrite(enable2_pin, M2_speed);
            delay(500);
            homingState = WAIT_AFTER_RIGHT;
            break;

        case WAIT_AFTER_RIGHT:
            // delay elapsed, idle briefly then move to bottom
            HomingIdle();
            homingState = MOVE_TO_BOTTOM;
            break;

        case MOVE_TO_BOTTOM:
            if (sB_flag) {
            //if (last_pressed == sB) {
                sB_flag = false;
                HomingIdle();
                homingState = MOVE_UP;
            } else {
                digitalWrite(motor1_pin, HIGH);
                digitalWrite(motor2_pin, LOW);
                analogWrite(enable1_pin, M1_speed);
                analogWrite(enable2_pin, M2_speed);
            }
            break;

        case MOVE_UP:
            // delay elapsed, move up briefly
            digitalWrite(motor1_pin, LOW);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, M1_speed);
            analogWrite(enable2_pin, M2_speed);
            delay(500);
            homingState = HOMING_COMPLETE;
            break;

        case HOMING_COMPLETE:
            HomingIdle();
            break;
    }
}
