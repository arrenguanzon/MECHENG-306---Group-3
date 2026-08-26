
#include "gcode.h"




#define GANTRY_WIDTH 205
#define GANTRY_HEIGHT 135




GCode::GCode(String &rc, float& abs_x, float& abs_y) : absoluteX(abs_x), absoluteY(abs_y) {
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
    unsigned int start = 0;
    bool valid = true;




    //split rc after ;
    int end = rc.indexOf(';');




    String rc_filtered;
    if (end >= 0) {
        rc_filtered = rc.substring(0, end);
    } else {
        rc_filtered = rc;
    }
   
     rc_filtered.trim();




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




        // A token ends at a space, the start of another command or the end of the string




        if(i>start) {
            String token = rc_filtered.substring(start, i);




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
        // If the command itself is valid, check the resulting position
        if (valid_command) {
            valid_command = checkBounds();
        }
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




    //inheriting speed --> NEED TO TEST!!!!
    if(command == G1 && valid_command) {
        if(has_speed) {
            previous_speed = speed_target;
        } else {
            speed_target = previous_speed;
        }
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




    unsigned int start = 1;




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




    for (unsigned int i = start; i < token.length(); i++) {
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
        speed_target = FeedrateToPWM(value);
        has_speed = true;
    }




    return true;
}








void GCode::printErrorCommand() const {
    if (!valid_command) {
        Serial.println("Invalid command: " + raw_command);
    }
}




bool GCode::checkBounds() {
    float newX = absoluteX;
    float newY = absoluteY;




    // G1 movements are RELATIVE
    if (has_x) {
        newX += x_target;
    }




    if (has_y) {
        newY += y_target;
    }




    if (newX < 0.0f || newX > GANTRY_WIDTH) {
        Serial.println("Error: X movement exceeds gantry limits.");
        Serial.print("Requested X position: ");
        Serial.println(newX);
        Serial.print("Allowed range: 0 - ");
        Serial.println(GANTRY_WIDTH);




        return false;
    }




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


float GCode::FeedrateToPWM(float feedrate) {
    // from data sheet: 33RPM for 6v
    // at 9v, rpm = 33 * (9/6) = 49.5RPM
    // feedrate is in mm/min
    // RPM needed = feedrate / circumference (pi * 15)
    // PWM value = (RPM / 49.5) * 255
    float pwmValue = ((feedrate / (PI * 15)) / 49.5) * 255.0f;
    if (pwmValue > 180.0f) {
        pwmValue = 180.0f;
    } else if (pwmValue < 0.0f) {
        pwmValue = 0.0f;
    }
    return pwmValue;
}


