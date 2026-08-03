#include <Arduino.h>


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


void TopISR(){


}


void BottomISR(){
    if(state == HOMING && last_pressed == START){
        // move towards left
    } 
    // else if (state == HOMING & ) {
    //     // 
    // }
}


void LeftISR(){
    if(state == HOMING && last_pressed == START){
        // move towards bottom
    }
}


void RightISR(){

}


void setup(){
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);


    pinMode(switch_top, INPUT_PULLUP);
    pinMode(switch_bottom, INPUT_PULLUP);
    pinMode(switch_left, INPUT_PULLUP);
    pinMode(switch_right, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switch_top), TopISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, LOW);
    attachInterrupt(digitalPinToInterrupt(switch_right), RightISR, LOW);
}
void Homing(){ // bummer do it again
    
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, LOW);
    analogWrite(enable1_pin, home_speed);
    analogWrite(enable2_pin, home_speed);


}

void loop(){
   Homing();
}





void IdleState(){
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}
