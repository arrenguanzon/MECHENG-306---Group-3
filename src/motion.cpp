#include <motion.h>
#include <Arduino.h>

#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

int ratio = 1.3;

motion::motion()
{
   
}

void motion::horizontalRight(int speed)
{
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, (speed * ratio));
    analogWrite(enable2_pin, speed);
}

void motion::horizontalLeft(int speed)
{
    digitalWrite(motor1_pin, LOW);
    digitalWrite(motor2_pin, LOW);
    analogWrite(enable1_pin, (speed * ratio));
    analogWrite(enable2_pin, speed);
}

void motion::verticalUp(int speed)
{
    digitalWrite(motor1_pin, LOW);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, (speed * ratio));
    analogWrite(enable2_pin, speed);
}

void motion::verticalDown(int speed)
{
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, LOW);
    analogWrite(enable1_pin, (speed * ratio));
    analogWrite(enable2_pin, speed);
}