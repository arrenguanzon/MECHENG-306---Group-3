#include <Arduino.h>
#include <encoder.h>
#include <motion.h>


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


// int M2_speed = 75;
// int M1_speed = M2_speed * 1.3;


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;

//function prototypes

void TopISR();
void BottomISR();
void LeftISR();
void RightISR();
void Homing();

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

void Idle(){
   digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(1000);
}

void BottomISR(){
    last_pressed = sB;
}

ISR(PCINT0_vect) {
   last_pressed = sT;
}

ISR(PCINT2_vect) {
    last_pressed = sR;
}

void LeftISR(){
    last_pressed = sL;
    // Serial.print("last_pressed: ");
    // Serial.println("Left switch pressed");
    // Idle();
}



void loop(){
    // last_pressed = START;
    // Homing();
    // Serial.println("Homing Complete");
    // while(1);

    encoderObject.move(10,10);
}


// void loop()
// {
//     digitalWrite(motor1_pin, LOW);
//     digitalWrite(motor2_pin, LOW);
//     analogWrite(enable1_pin, 100);
//     analogWrite(enable2_pin, 100);
// }


// void Homing(){
//     while((last_pressed == START) | (last_pressed == sT) | (last_pressed == sB) | (last_pressed == sR)){
//         Serial.println(last_pressed);
//         if(last_pressed == sB){
//             digitalWrite(motor1_pin, LOW);
//             digitalWrite(motor2_pin, HIGH);
//             analogWrite(enable1_pin, M1_speed);
//             analogWrite(enable2_pin, M2_speed); 
//             delay(1000);
//         }
//         digitalWrite(motor1_pin, LOW);
//         digitalWrite(motor2_pin, LOW);
//         analogWrite(enable1_pin, M1_speed);
//         analogWrite(enable2_pin, M2_speed);

//     }
    
//     Idle();
//     // implement logic to move to the right
//     digitalWrite(motor1_pin, HIGH);
//     digitalWrite(motor2_pin, HIGH);
//     analogWrite(enable1_pin, M1_speed);
//     analogWrite(enable2_pin, M2_speed);
//     delay(500);
//     Idle();

//     while((last_pressed == START) | (last_pressed == sT) | (last_pressed == sL) | (last_pressed == sR)){
//         Serial.println(last_pressed);
//         digitalWrite(motor1_pin, HIGH);
//         digitalWrite(motor2_pin, LOW);
//         analogWrite(enable1_pin, M1_speed);
//         analogWrite(enable2_pin, M2_speed);
//     }
//     Idle();
// }