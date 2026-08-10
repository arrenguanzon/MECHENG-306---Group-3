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


// Homing base speeds
int M2_speed = 100;
int M1_speed = M2_speed * 1.3;


typedef enum SwitchState {sT, sB, sL, sR, START} SwitchState;
typedef enum States {IDLE, HOMING, MOVING, FAULT} States;
volatile States state = IDLE;
volatile SwitchState last_pressed = START;

//function prototypes
void BottomISR();
void LeftISR();
void Homing();
void Idle();
void encoder1();
void encoder2();


//objects

encoder encoderObject;

//setup function
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
    
    //setup for encoders
    attachInterrupt(digitalPinToInterrupt(ENCODER1_A), encoder1, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(ENCODER1_B), encoder1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_A), encoder2, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(ENCODER2_B), encoder2, CHANGE);

}

//loop function
void loop()
{
    // digitalWrite(motor1_pin, LOW);
    // digitalWrite(motor2_pin, HIGH);
    // analogWrite(enable1_pin, M1_speed);
    // analogWrite(enable2_pin, M2_speed);

    encoderObject.move(10, 10);
    delay(1000); //for testing purposes, to see if the motors move to the desired position
}

//----------------------------------------------------------------------------------------------------------------------------------//

//interrupt service routines for limit switches ( will need to add idle stuff at some point and switch debouncing)

void BottomISR(){
    last_pressed = sB;
}
void LeftISR(){
    last_pressed = sL;
}

//top switch and right switch are handled by pin change interrupts
ISR(PCINT0_vect) {
   last_pressed = sT;
}

ISR(PCINT2_vect) {
    last_pressed = sR;
}

//encoder interrupt service routines [check to make sure they correlate with correct encoders and directions]
void encoder1()
{
    // check direction of encoder
    if (digitalRead(ENCODER1_A) == digitalRead(ENCODER1_B)){
        encoderObject.currentCountA--;
        encoderObject.distanceFromOriginx--;
    }
    else {
        encoderObject.currentCountA++;
        encoderObject.distanceFromOriginx++;
    }
}

void encoder2()
{
    // check direction of encoder
    if (digitalRead(ENCODER2_A) == digitalRead(ENCODER2_B)){
        encoderObject.currentCountB--;
        encoderObject.distanceFromOriginy--;
    }
    else{
        encoderObject.currentCountB++;
        encoderObject.distanceFromOriginy++;
    }
}



//placeholder idle
void Idle(){
    digitalWrite(motor1_pin, 0);
    digitalWrite(motor2_pin, 0);
    analogWrite(enable1_pin, 0);
    analogWrite(enable2_pin, 0);
    delay(1000);
}

//placeholder place for homing function might put it somewhere related to FSM later

void Homing(){
    while((last_pressed == START) | (last_pressed == sT) | (last_pressed == sB) | (last_pressed == sR)){
        Serial.println(last_pressed);
        if(last_pressed == sB){
            digitalWrite(motor1_pin, LOW);
            digitalWrite(motor2_pin, HIGH);
            analogWrite(enable1_pin, M1_speed);
            analogWrite(enable2_pin, M2_speed); 
            delay(1000);
        }
        digitalWrite(motor1_pin, LOW);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);

    }
    
    Idle();
    // implement logic to move to the right
    digitalWrite(motor1_pin, HIGH);
    digitalWrite(motor2_pin, HIGH);
    analogWrite(enable1_pin, M1_speed);
    analogWrite(enable2_pin, M2_speed);
    delay(500);
    Idle();

    while((last_pressed == START) | (last_pressed == sT) | (last_pressed == sL) | (last_pressed == sR)){
        Serial.println(last_pressed);
        digitalWrite(motor1_pin, HIGH);
        digitalWrite(motor2_pin, LOW);
        analogWrite(enable1_pin, M1_speed);
        analogWrite(enable2_pin, M2_speed);
    }
    Idle();
}



