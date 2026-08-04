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

void setup(){
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);
}

void loop(){
    digitalWrite(motor1_pin, HIGH);
    analogWrite(enable1_pin, 100);
}

/*
void Homing(){
    while(last_pressed == START){
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, home_speed);
        analogWrite(enable2_pin, home_speed);
    }
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    _delay_ms(500);
    while(last_pressed == sL){
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, home_speed);
        analogWrite(enable2_pin, home_speed);
    }
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
}

void loop(){
   Homing();
   while(1);
}

*/