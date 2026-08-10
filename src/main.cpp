#include <Arduino.h>
#include <encoder.h>
#include <gcode.h>


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

String user_input = "";

// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;


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
void ENCODER1AISR();
void ENCODER1BISR();
void ENCODER2AISR();
void ENCODER2BISR();

// encoder encoderObject;


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
    // Set up interrupts for limit switches
    pinMode(ENCODER1_A, INPUT_PULLUP);
    pinMode(ENCODER1_B, INPUT_PULLUP);
    pinMode(ENCODER2_A, INPUT_PULLUP);
    pinMode(ENCODER2_B, INPUT_PULLUP);


    // Attach interrupts for limit switches
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
    //Top switch
    PCICR |= (1 << PCIE0); 
    PCMSK0 |= (1 << PCINT4);
    //Right switch
    PCICR |= (1 << PCIE2);      
    PCMSK2 |= (1 << PCINT16);
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);

    // Attach interrupts for encoders
    attachInterrupt(digitalPinToInterrupt(ENCODER1_A), ENCODER1AISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER1_B), ENCODER1BISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_A), ENCODER2AISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_B), ENCODER2BISR, CHANGE);

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

void ENCODER1AISR() {
    
}

void ENCODER1BISR() {
    
}

void ENCODER2AISR() {
    
}

void ENCODER2BISR() {
    
}

void loop(){

    // Input reading
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
