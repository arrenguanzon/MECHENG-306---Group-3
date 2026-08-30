#include <Arduino.h>
#include "encoder.h"
#include "gcode.h"
#include "fsm.h"
#include "motionController.h"

// Motor pins
#define motor1_pin 7
#define enable1_pin 6
#define enable2_pin 5
#define motor2_pin 4

// Limit switch pins
#define switch_top 10
#define switch_bottom 3
#define switch_left 2
#define switch_right A8

// Encoder pins (quadrature encoders)
#define ENCODER1_A 18
#define ENCODER1_B 19
#define ENCODER2_A 20
#define ENCODER2_B 21

// Switch debouncing parameters
#define DEBOUNCE_MS 50
unsigned long last_sB_time = 0;
unsigned long last_sT_time = 0;
unsigned long last_sL_time = 0;
unsigned long last_sR_time = 0;

// Fixed frequency control loop 
#define CONTROL_FREQUENCY_HZ 100
#define CONTROL_INTERVAL_MS (1000UL / CONTROL_FREQUENCY_HZ)
unsigned long lastControlUpdate = 0;

// Track last pressed limit switch for homing
volatile MotionController::SwitchState last_pressed =
    MotionController::START;

// Absolute position in cartesian coordinates (mm)
float absoluteX = 0.0f;
float absoluteY = 0.0f;

// System objects
Encoder encoder;
MotionController motionController(encoder, absoluteX, absoluteY, last_pressed);

String user_input = "";
FSM fsm(motionController, last_pressed);

// Function prototypes for interrupt service routines
void TopISR();
void BottomISR();
void LeftISR();
void RightISR();
void ENCODER1AISR();
void ENCODER1BISR();
void ENCODER2AISR();
void ENCODER2BISR();


void setup()
{
    // Configure motor control pins as outputs
    pinMode(motor1_pin, OUTPUT);
    pinMode(motor2_pin, OUTPUT);

    // Initialise serial communication for user input/debug
    Serial.begin(9600);
    
    // Configure limit switch pins with pull-up resistors
    pinMode(switch_top, INPUT_PULLUP);
    pinMode(switch_bottom, INPUT_PULLUP);
    pinMode(switch_left, INPUT_PULLUP);
    pinMode(switch_right, INPUT_PULLUP);
    
    // Configure encoder pins with pull-up resistors
    pinMode(ENCODER1_A, INPUT_PULLUP);
    pinMode(ENCODER1_B, INPUT_PULLUP);
    pinMode(ENCODER2_A, INPUT_PULLUP);
    pinMode(ENCODER2_B, INPUT_PULLUP);

    // Attach external interrupts for limit switches
    attachInterrupt(digitalPinToInterrupt(switch_bottom), BottomISR, FALLING);
    
    // Top switch: Pin Change Interrupt
    PCICR |= (1 << PCIE0); 
    PCMSK0 |= (1 << PCINT4);
    
    // Right switch: Pin Change Interrupt
    PCICR |= (1 << PCIE2);      
    PCMSK2 |= (1 << PCINT16);
    
    // Left switch: External Interrupt
    attachInterrupt(digitalPinToInterrupt(switch_left), LeftISR, FALLING);

    // Attach external interrupts for encoders (trigger on any change)
    attachInterrupt(digitalPinToInterrupt(ENCODER1_A), ENCODER1AISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER1_B), ENCODER1BISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_A), ENCODER2AISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_B), ENCODER2BISR, CHANGE);

    // Print welcome message and instructions
    Serial.println("=== Welcome to our XY-Plotter! ===");
    Serial.println("Please enter Homing command (G28) before moving the plotter with G1 command.");
    Serial.println("Gantry Limit: X:205mm, Y:135mm");
    
}

void loop(){
    // Read and process serial input from user
    while (Serial.available() > 0)
    {
        char c = Serial.read();

        // Check for line termination 
        if (c == '\n' || c == '\r') {
            if (user_input.length() > 0)
            {
                Serial.println();
                // Parse and execute G-code command
                GCode gcode(user_input, absoluteX, absoluteY);
                fsm.processCommand(gcode);

                Serial.print("State: ");
                Serial.println(fsm.getStateName());

                user_input = "";
            }
        }
        // Allow for backspace and delete
        else if (c == '\b' || c == (char)127) {
            if (user_input.length() > 0) {
                // Remove the last character from the buffer string
                user_input.remove(user_input.length() - 1);
                
                // Erase the character from the serial monitor
                Serial.print("\b \b");
            }
        }
        else {
            Serial.print(c);   // Echo character immediately
            user_input += c;
        }
    }
    
    // Fixed frequency control loop: execute motion control at constant rate
    unsigned long now = millis();
    if (now - lastControlUpdate >= CONTROL_INTERVAL_MS) {
        lastControlUpdate = now;
        fsm.update();  // Update FSM 
    }
}

// Limit Switch ISR's

// Bottom limit switch ISR
void BottomISR(){
    unsigned long now = millis();
    //  ignore if pressed within DEBOUNCE_MS of last press
    if((digitalRead(switch_bottom) == LOW && (now - last_sB_time >= DEBOUNCE_MS)) ) {
        last_sB_time = now;
        last_pressed = MotionController::sB;
        // During homing, bottom switch press is expected, during movement it's a fault
        if (fsm.getState() != FSM::HOMING) {
            fsm.setState(FSM::FAULT);
        }
    }   
}

// Top limit switch ISR (Pin Change Interrupt)
ISR(PCINT0_vect) {
    unsigned long now = millis();
    // Debounce and check if top switch is pressed
    if ((digitalRead(switch_top) == LOW && (now - last_sT_time >= DEBOUNCE_MS)) ) { 
        last_sT_time = now;
        last_pressed = MotionController::sT;
        // Top switch press is always a fault (not part of homing)
        fsm.setState(FSM::FAULT);
    }
}

// Right limit switch ISR (Pin Change Interrupt)
ISR(PCINT2_vect) {
    unsigned long now = millis();
    // Debounce and check if right switch is pressed
    if ((digitalRead(switch_right) == LOW && (now - last_sR_time >= DEBOUNCE_MS)) ) { 
        last_sR_time = now;
        last_pressed = MotionController::sR;
        // Right switch press is always a fault
        fsm.setState(FSM::FAULT);
    }
}

// Left limit switch ISR
void LeftISR(){
    unsigned long now = millis();
    // ignore if pressed within DEBOUNCE_MS of last press
    if ((digitalRead(switch_left) == LOW && (now - last_sL_time >= DEBOUNCE_MS)) ) { 
        last_sL_time = now;
        last_pressed = MotionController::sL;
        // During homing, left switch press is expected, during movement it's a fault
        if (fsm.getState() != FSM::HOMING) {
            fsm.setState(FSM::FAULT);
        }
    }
}

// Encoder ISR's

// Motor 1, Channel A ISR
void ENCODER1AISR() {
    bool A = digitalRead(ENCODER1_A);
    bool B = digitalRead(ENCODER1_B);

    // If A and B are same, motor is moving backward
    if (A == B) {
        encoder.decrementMotor1Count();
    } else {
        encoder.incrementMotor1Count();
    }
}

// Motor 1, Channel B ISR
void ENCODER1BISR() {
    bool A = digitalRead(ENCODER1_A);
    bool B = digitalRead(ENCODER1_B);

    // If A and B are different, motor is moving backward
    if (A != B) {
        encoder.decrementMotor1Count();
    } else {
        encoder.incrementMotor1Count();
    }
}

// Motor 2, Channel A ISR
void ENCODER2AISR() {
    bool A = digitalRead(ENCODER2_A);
    bool B = digitalRead(ENCODER2_B);

    // If A and B are same, motor is moving backward
    if (A == B) {
        encoder.decrementMotor2Count();
    } else {
        encoder.incrementMotor2Count();
    }
}

// Motor 2, Channel B ISR
void ENCODER2BISR() {
    bool A = digitalRead(ENCODER2_A);
    bool B = digitalRead(ENCODER2_B);

    // If A and B are different, motor is moving backward
    if (A != B) {
        encoder.decrementMotor2Count();
    } else {
        encoder.incrementMotor2Count();
    }
}