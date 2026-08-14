#include "gcode.h"


GCode::GCode(String &rc){
    raw_command = rc;

    command = UNDEFINED;

    valid_command = false;

    has_x = false;
    has_y = false;
    has_speed = false;

    x_target = 0.0f;
    y_target = 0.0f;

    tokeniseInput(rc);
}

void GCode::tokeniseInput(const String &rc) {
    int start = 0;
    bool valid = true;
    for (int i = 0; i <= rc.length(); i++) {
        if (i == rc.length() || rc.charAt(i) == ' ') {
            String token = rc.substring(start, i);

            // Check for command tokens
            if (token == "G1" || token == "G01" || token == "g1" || token == "g01") {
                if (command == UNDEFINED) {
                command = G1;
                } else {
                    valid = false;
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

                    if (!parseValue(token)) {
                        valid = false;
                    }
                }
            }

            // If this character is a new one, don't skip it --> it normally skips spaces this is here so it dosen't skip commands.
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
    // Validity checks for commands
    if (!valid) {
        valid_command = false;
    } else if (command == G1) {
        // G1 must have at least one of X or Y, and can optionally have F
        valid_command = has_x || has_y;
    } else if (command == G28) {
        // G28 must not have X, Y or F
        valid_command = !has_x && !has_y && !has_speed;
    } else if (command == M999) {
        // M999 must not have X, Y or F
        valid_command = !has_x && !has_y && !has_speed;
    } else {
        valid_command = false;
    }

    // If the command is invalid, print an error message
    if (!valid_command) {
        printErrorCommand();
    }
}


bool GCode::parseValue(const String &token) {
    if (token.length() <= 1)
    {
        return false;
    }
    // Check if the first character is a valid type (X, Y, F)
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

    int start = 1;

    // Allow negative numbers
    if (token.charAt(start) == '-') {
        negative = true;
        start++;
    }

    // "-" by itself is not a number
    if (start == token.length()) {
        return false;
    }

    float value = 0;
    float decimalPlace = 0.1;

    for (int i = start; i < token.length(); i++) {
        char c = token.charAt(i);

        if (c >= '0' && c <= '9') {
            digitFound = true;


            if (!decimalFound) {
                value = value * 10 + (c - '0');
            } else {
                value = value + (c - '0') * decimalPlace;
                decimalPlace *= 0.1;
            }
        } else if (c == '.' && !decimalFound) {
            decimalFound = true;
        } else {
            return false;
        }
    }
    if (!digitFound) {
        return false;
    }

    if (negative) {
        value = -value;
    }

    // Store the value
    if (type == 'X') {
        if (has_x) {
            return false; // Duplicate X value
        }
        x_target = value;
        has_x = true;
    } else if (type == 'Y') {
        if (has_y) {
            return false; // Duplicate Y value
        }
        y_target = value;
        has_y = true;
    } else if (type == 'F') {
        if (has_speed) {
            return false; // Duplicate F value
        }
        speed_target = value;
        has_speed = true;
    }

    return true;
}


void GCode::printErrorCommand() const {
    if (!valid_command) {
        Serial.println("Invalid command: " + raw_command);
    }
}


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