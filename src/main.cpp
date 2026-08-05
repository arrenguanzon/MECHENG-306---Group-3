#include <Arduino.h>
#include <encoder.h>
#include <motion.h>


#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7


#define switch_top 16
#define switch_bottom 17
// #define switch_left 18
// #define switch_right 19

#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21


#define home_speed 100


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;


//our other .h files will too

//global consts and vars
bool atHome = 0;

//function prototypes

void TopISR();
void BottomISR();
void LeftISR();
void RightISR();
void encoder1();
void encoder2();

//objects

encoder encoderObject;


void setup()
{
    // Set up motor pins
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);

    Serial.begin(9600);
    // Set up limit switch pins
    pinMode(switch_top, INPUT_PULLUP);
    pinMode(switch_bottom, INPUT_PULLUP);
    // pinMode(switch_left, INPUT_PULLUP);
    // pinMode(switch_right, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switch_top), TopISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
    // attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);
    // attachInterrupt(digitalPinToInterrupt(switch_right), RightISR, FALLING);

    attachInterrupt(digitalPinToInterrupt(ENCODER1_A), encoder1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_A), encoder2, CHANGE);

}


void TopISR(){
    last_pressed = sT;
    state = IDLE;
    Serial.print("last_pressed: ");
    Serial.println(last_pressed);
    Serial.println("Top switch pressed");

}
void BottomISR(){
    last_pressed = sB;
    state = IDLE;
    Serial.print("last_pressed: ");
    Serial.println(last_pressed);
    Serial.println("Bottom switch pressed");
}
void LeftISR(){
    last_pressed = sL;
    state = IDLE;
    Serial.print("last_pressed: ");
    Serial.println("Left switch pressed");
}
void RightISR(){
    last_pressed = sR;
    state = IDLE;
    Serial.print("last_pressed: ");
    Serial.println(last_pressed);
    Serial.println("Right switch pressed");
}

void loop()
{
    // digitalWrite(motor1_pin, LOW);
    // digitalWrite(motor2_pin, LOW);
    // analogWrite(enable1_pin, 100);
    // analogWrite(enable2_pin, 100);

    encoderObject.moveTo(100, 100);
    delay(1000); //for testing purposes, to see if the motors move to the desired position
}

void encoder1()
{

    // check direction of encoder
    if (digitalRead(ENCODER1_A) == digitalRead(ENCODER1_B))
    {
        // moving forward
        encoderObject.currentCountA++;
    }
    else
    {
        // moving backward
        encoderObject.currentCountA--;
    }
}

void encoder2()
{
    // check direction of encoder
    if (digitalRead(ENCODER2_A) == digitalRead(ENCODER2_B))
    {
        // moving forward
        encoderObject.currentCountB++;
    }
    else
    {
        // moving backward
        encoderObject.currentCountB--;
    }
}



// interrupts

//function implementations if we have any in the main

