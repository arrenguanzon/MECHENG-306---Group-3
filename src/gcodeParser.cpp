#include "gcodeParser.h"
#include "config.h"

#include <Arduino.h>

void tokeniseInput(String gcode_string, String tokens[]) {
    // Break up G-code string into tokens based on spaces

    // Place the values into the global tokens array for later use
}

void extractParameters(String tokens[], MovementType &movementType, float &xOffset, float &yOffset, float &motionSpeed) {
    // Extract the parameters from the tokens and assign them to the appropriate variables
}

void trackPosition(float &currentXPosition, float &currentYPosition, float &xOffset, float &yOffset, float &newXPosition, float &newYPosition) {
    // If G28 or M999 is received, the new position will be set to 0,0

    // Determine the new position of the system based on the current position and the offsets provided by the G-code command
}

bool validateMove(MovementType &movementType, float &motionSpeed, float &xOffset, float &yOffset, float &newXPosition, float &newYPosition) {
    // If G28 or M999 is received, the move is always valid

    // Do all the checks
    // Use my brain to think of all checks needed
    // Then use AI to generate more :( just to be sure
}
