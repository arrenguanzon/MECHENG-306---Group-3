#include "gcode.h"

// Gantry dimensions (mm)
#define GANTRY_WIDTH 205
#define GANTRY_HEIGHT 135

// Constructor: Parse G-code string and initialise 
GCode::GCode(String &rc, float& abs_x, float& abs_y) : absoluteX(abs_x), absoluteY(abs_y) {
    raw_command = rc;
    command = UNDEFINED;
    valid_command = false;

    has_x = false;
    has_y = false;
    has_speed = false;

    x_target = 0.0f;
    y_target = 0.0f;

    // Tokenise and parse the input string
    tokeniseInput(rc);
}

// Tokenise input string and extract G-code command and parameters
void GCode::tokeniseInput(const String &rc) {
    unsigned int start = 0;
    bool valid = true;

    // Remove comments (everything after semicolon)
    int end = rc.indexOf(';');

    String rc_filtered;
    if (end >= 0) {
        rc_filtered = rc.substring(0, end);
    } else {
        rc_filtered = rc;
    }
   
    rc_filtered.trim();

    // Remove all spaces for easier parsing
    String temp = "";
    temp.reserve(rc_filtered.length());
    
    for (unsigned int i = 0; i < rc_filtered.length(); i++) {
        char c = rc_filtered.charAt(i);
        if (c != ' ') {
            temp += c;
        }
    }
    rc_filtered = temp;

    // Tokenise by splitting on command/parameter characters (G, X, Y, F, M)
    for (unsigned int i = 0; i <= rc_filtered.length(); i++) {
        if (i == rc_filtered.length() ||
            rc_filtered.charAt(i) == ' ' ||
            rc_filtered.charAt(i) == 'X' ||
            rc_filtered.charAt(i) == 'x' ||
            rc_filtered.charAt(i) == 'Y' ||
            rc_filtered.charAt(i) == 'y' ||
            rc_filtered.charAt(i) == 'F' ||
            rc_filtered.charAt(i) == 'f' ||
            rc_filtered.charAt(i) == 'G' ||
            rc_filtered.charAt(i) == 'g' ||
            rc_filtered.charAt(i) == 'M' ||
            rc_filtered.charAt(i) == 'm') {

        // Extract token 
        if(i>start) {
            String token = rc_filtered.substring(start, i);

            // Identify command tokens
            if (token == "G1" || token == "G01" || token == "g1" || token == "g01") {
                if (command == UNDEFINED) {
                command = G1;
                } else {
                    valid = false;  // Multiple commands not allowed
                }
            } else if (token == "G28" || token == "g28") {
                if (command == UNDEFINED) {
                    command = G28;
                } else {
                    valid = false;
                }
            } else if (token == "M999" || token == "m999") {
                if (command == UNDEFINED) {
                    command = M999;
                } else {
                    valid = false;
                }
            } else {
                    // Try to parse as parameter (X, Y, or F)
                    if (!parseValue(token)) {
                        valid = false;
                    }
                }  
            }

            // Update start position for next token (don't skip command characters)
            if (rc_filtered.charAt(i) == 'X' ||
                rc_filtered.charAt(i) == 'x' ||
                rc_filtered.charAt(i) == 'Y' ||
                rc_filtered.charAt(i) == 'y' ||
                rc_filtered.charAt(i) == 'F' ||
                rc_filtered.charAt(i) == 'f' ||
                rc_filtered.charAt(i) == 'G' ||
                rc_filtered.charAt(i) == 'g' ||
                rc_filtered.charAt(i) == 'M' ||
                rc_filtered.charAt(i) == 'm') {

                start = i;
            } else {
                start = i + 1;
            }
        }
    }


    // Validity checks: ensure command has correct parameters
    if (!valid) {
        valid_command = false;
    } else if (command == G1) {
        // G1 must have at least one of X or Y, optionally F for speed
        valid_command = has_x || has_y;
        // If command is valid, check if movement is within gantry bounds
        if (valid_command) {
            valid_command = checkBounds();
        }
    } else if (command == G28) {
        // G28 (homing) must not have X, Y or F parameters
        valid_command = !has_x && !has_y && !has_speed;
    } else if (command == M999) {
        // M999 (reset) must not have X, Y or F parameters
        valid_command = !has_x && !has_y && !has_speed;
    } else {
        valid_command = false;
    }

    // If invalid, print error message
    if (!valid_command) {
        printErrorCommand();
    }

    // Speed inheritance: if G1 command doesn't specify speed, use previous speed
    if(command == G1 && valid_command) {
        if(has_speed) {
            previous_speed = speed_target;
        } else {
            speed_target = previous_speed;
        }
    }
}

// Parse a parameter token (X, Y, or F)
bool GCode::parseValue(const String &token) {
    // Minimum token length is 2 (letter + digit)
    if (token.length() <= 1)
    {
        return false;
    }

    // Extract parameter type (X, Y, or F)
    char type = token.charAt(0);
    if (type == 'X' || type == 'x') {
        type = 'X';
    } else if (type == 'Y' || type == 'y') {
        type = 'Y';
    } else if (type == 'F' || type == 'f') {
        type = 'F';
    } else {
        return false;
    }

    bool decimalFound = false;
    bool digitFound = false;
    bool negative = false;

    unsigned int start = 1;

    // Allow negative numbers
    if (token.charAt(start) == '-') {
        negative = true;
        start++;
    }

    // "-" by itself is not a valid number
    if (start == token.length()) {
        return false;
    }

    // Parse number (integer and decimal parts)
    float value = 0;
    float decimalPlace = 0.1;

    for (unsigned int i = start; i < token.length(); i++) {
        char c = token.charAt(i);

        if (c >= '0' && c <= '9') {
            digitFound = true;

            if (!decimalFound) {
                // Integer part
                value = value * 10 + (c - '0');
            } else {
                // Decimal part
                value = value + (c - '0') * decimalPlace;
                decimalPlace *= 0.1;
            }

        } else if (c == '.' && !decimalFound) {
            // First decimal point is allowed
            decimalFound = true;
        } else {
            // Invalid character
            return false;
        }
    }

    // At least one digit must be present
    if (!digitFound) {
        return false;
    }

    // Apply negative sign if present
    if (negative) {
        value = -value;
    }

    // Round decimal values
    if(decimalFound){ 
        value = round(value);
    }

    // Store the parsed value in appropriate field
    if (type == 'X') {
        if (has_x) {
            return false; // Duplicate X parameter not allowed
        }
        x_target = value;
        has_x = true;
    } else if (type == 'Y') {
        if (has_y) {
            return false; // Duplicate Y parameter not allowed
        }
        y_target = value;
        has_y = true;
    } else if (type == 'F') {
        if (has_speed) {
            return false; // Duplicate F parameter not allowed
        }
        speed_target = FeedrateToPWM(value);
        has_speed = true;
    }
    return true;
}

// Print error message if command is invalid
void GCode::printErrorCommand() const {
    if (!valid_command) {
        Serial.println("Invalid command: " + raw_command);
    }
}

// Check if target position is within gantry workspace bounds
// G1 movements are relative, so we add to current position
bool GCode::checkBounds() {
    float newX = absoluteX;
    float newY = absoluteY;

    // Calculate final position (G1 movements are relative)
    if (has_x) {
        newX += x_target;
    }

    if (has_y) {
        newY += y_target;
    }

    // Check X bounds
    if (newX < 0.0f || newX > GANTRY_WIDTH) {
        Serial.println("Error: X movement exceeds gantry limits.");
        Serial.print("Requested X position: ");
        Serial.println(newX);
        Serial.print("Allowed range: 0 - ");
        Serial.println(GANTRY_WIDTH);

        return false;
    }

    // Check Y bounds
    if (newY < 0.0f || newY > GANTRY_HEIGHT) {
        Serial.println("Error: Y movement exceeds gantry limits.");
        Serial.print("Requested Y position: ");
        Serial.println(newY);
        Serial.print("Allowed range: 0 - ");
        Serial.println(GANTRY_HEIGHT);

        return false;
    }

    return true;
}

// Getter functions for command parameters
GCode::Command GCode::getCommand() const {
    return command;
}

float GCode::getXTarget() const {
    return x_target;
}

float GCode::getYTarget() const {
    return y_target;
}

float GCode::getSpeedTarget() const {
    return speed_target;
}

bool GCode::isValid() const {
    return valid_command;
}

// Convert feedrate (mm/min) to motor PWM value (0-255)
// Based on motor specification: 33 RPM at 6V
// At 9V: RPM = 33 * (9/6) = 49.5 RPM
// Wheel diameter: 15mm, circumference: PI * 15mm
float GCode::FeedrateToPWM(float feedrate) {
    // Calculate required RPM from feedrate
    float pwmValue = ((feedrate / (PI * 14.3)) / 49.5) * 255.0f;
    
    // Clamp to motor PWM range
    if (pwmValue > 180.0f) {
        pwmValue = 180.0f;
    } else if (pwmValue < 0.0f) {
        pwmValue = 0.0f;
    }
    return pwmValue;
}