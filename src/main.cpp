#include <Arduino.h>
#include <encoder.h>


#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7


#define switch_top 16
#define switch_bottom 17
#define switch_left 18
#define switch_right 19


#define home_speed 100


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;
String user_input = "";


//Other .h files will too
#include "gcode.h"

//global consts and vars
bool atHome = 0;

//function prototypes

void moveTo(int x, int y);
void TopISR();
void BottomISR();
void LeftISR();
void RightISR();


void setup()
{
    // Set up motor pins
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);

    // For Serial Printing, and Input Channels
    Serial.begin(9600);
    Serial.println("Serial Communication Started");
    // Set up limit switch pins
    pinMode(switch_top, INPUT_PULLUP);
    pinMode(switch_bottom, INPUT_PULLUP);
    pinMode(switch_left, INPUT_PULLUP);
    pinMode(switch_right, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switch_top), TopISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch_right), RightISR, FALLING);
}


// ISR Interrupts
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

void loop() {
    digitalWrite(motor1_pin, LOW);
    digitalWrite(motor2_pin, LOW);
    analogWrite(enable1_pin, 100);
    analogWrite(enable2_pin, 100);

    // Input reading
    // while (Serial.available() > 0) {
    //     char c = Serial.read();

    //     if (c == '\n' || c == '\r') {

    //         if (user_input.length() > 0) {

    //             Serial.print("Received: ");
    //             Serial.println(user_input);

    //             GCode gcode(user_input);

    //             user_input = "";
    //         }
    //     }
    //     else {
    //         user_input += c;
    //     }
    // }
    while (Serial.available() > 0)
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (user_input.length() > 0)
            {
                Serial.println();

                GCode gcode(user_input );

                user_input = "";
            }
        }
        else
        {
            Serial.print(c);   // Echo character immediately
            user_input += c;
        }
    }
}


//function implementations if we have any in the main

void moveTo(int x, int y)
{

}
