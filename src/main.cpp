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
