#include <Arduino.h>

#include "config.h"
#include "gcodeParser.h"

volatile States state = IDLE; // IDLE is the default state

// Current and new positions of the system
float currentXPosition = 0;
float currentYPosition = 0;
float newXPosition = 0;
float newYPosition = 0;

// G-code command input variables
MovementType movementType;
float xOffset = 0;
float yOffset = 0;
float motionSpeed = 0; // Motion speed is inherited (if a command does not specify a speed, the previous speed is used)

// G-code parsing variables
String tokens[4];
bool isValidMove = false;

void setup()
{
   
}

void loop()
{
    switch (state) {
    case IDLE:
        // Read user input and change to the appropriate state (HOMING, MOVING)
        // Double check that Serial.available does what I want !!
        if (Serial.available() > 0) { // Check if data is available to read
            // Read the input string until semicolon character
            String gcode_string = Serial.readStringUntil(';');  // Double check that this function does what I want !!

            // Turn the input string into meaningful tokens
            tokeniseInput(gcode_string, tokens); // (arrays are automatically passed by reference)
            // Extract parameters from the tokens
            extractParameters(tokens, movementType, xOffset, yOffset, motionSpeed);
            
            // Determine where the movement command will take the system
            trackPosition(currentXPosition, currentYPosition, xOffset, yOffset, newXPosition, newYPosition);
            // Determine if the move is valid
            isValidMove = validateMove(movementType, motionSpeed, xOffset, yOffset, newXPosition, newYPosition);

            // If the movement is valid, execute each movement command in the G-code string
            if (isValidMove) {
                if (movementType == G01) {
                    // Draw line
                } else if (movementType == G28) {
                    // Homing command
                } else if (movementType == M999) {
                    // Reset
                }
            }
        }
        
        // Interpret errors and change to FAULT state
        break;
    case HOMING:
        // Move to home position

        // Change to IDLE state
        break;  
    case MOVING:
        // Move to target position

        // Change to IDLE state
        break;
    case FAULT:
        // Parse G-code input and move to IDLE state when M999 is received
        break;
    }
}
#include <Arduino.h>
#include "encoder.h"
#include "gcode.h"
#include "fsm.h"
#include "motionController.h"

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


volatile MotionController::SwitchState last_pressed =
    MotionController::START;

// Absolution position tracker and Initialising Motion Controller
float absoluteX = 0.0f;
float absoluteY = 0.0f;

Encoder encoder;
MotionController motionController(encoder, absoluteX, absoluteY);

String user_input = "";
FSM fsm(motionController, last_pressed);

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

void loop(){
    // Input reading
    while (Serial.available() > 0)
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (user_input.length() > 0)
            {
                Serial.println();
                GCode gcode(user_input);

                fsm.processCommand(gcode);

                Serial.print("State: ");
                Serial.println(fsm.getStateName());

                user_input = "";
            }
        }
        else {
            Serial.print(c);   // Echo character immediately
            user_input += c;
        }
    }
    fsm.update();
}

void BottomISR(){
    last_pressed = MotionController::sB;
}

ISR(PCINT0_vect) { // Top Limit Switch
   last_pressed = MotionController::sT;
}

ISR(PCINT2_vect) { // Right Limit Switch
    last_pressed = MotionController::sR;
}

void LeftISR(){
    last_pressed = MotionController::sL;
    // Serial.print("last_pressed: ");
    // Serial.println("Left switch pressed");
    // Idle();
}

void ENCODER1AISR() {
    bool A = digitalRead(ENCODER1_A);
    bool B = digitalRead(ENCODER1_B);

    if (A == B) {
        encoder.incrementMotor1Count();
    } else {
        encoder.decrementMotor1Count();
    }
}

void ENCODER1BISR() {
    bool A = digitalRead(ENCODER1_A);
    bool B = digitalRead(ENCODER1_B);

    if (A != B) {
        encoder.incrementMotor1Count();
    } else {
        encoder.decrementMotor1Count();
    }
}

void ENCODER2AISR() {
    bool A = digitalRead(ENCODER2_A);
    bool B = digitalRead(ENCODER2_B);

    if (A == B) {
        encoder.incrementMotor2Count();
    } else {
        encoder.decrementMotor2Count();
    }
}

void ENCODER2BISR() {
    bool A = digitalRead(ENCODER2_A);
    bool B = digitalRead(ENCODER2_B);

    if (A != B) {
        encoder.incrementMotor2Count();
    } else {
        encoder.decrementMotor2Count();
    }
}


