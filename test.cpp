#include <iostream>
#include <Arduino.h>
#include <cmath.h>


#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7


#define switch_top 2
#define switch_bottom 3
#define switch_left 18
#define switch_right 19


#define home_speed 100


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;


void setup(){
    pinmode(motor1_pin, OUTPUT);
    pinmode(motor2_pin, OUTPUT);


    pinmode(switch_top, INPUT_PULLUP);
    pinmode(switch_bottom, INPUT_PULLUP);
    pinmode(switch_left, INPUT_PULLUP);
    pinmode(switch_right, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switch_top), TopISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_right), RightISR, LOW);
}


void loop(){
    switch (state == HOMING) {
       
    }
}


void TopISR(){


}


void BottomISR(){
    if(state == HOMING & last_pressed == START){
        //move towards left
    } else if (state == HOMING & )
}


void LeftISR(){
    if(state == HOMING & last_pressed = START){
        //move towards bottom
    }
}


void RightISR(){


}


void Homing(){ // bummer do it again
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, LOW);
    analogWrite(enable1_pin, home_speed);
    analogWrite(enable2_pin, home_speed);


}


void IDLE(){
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}
