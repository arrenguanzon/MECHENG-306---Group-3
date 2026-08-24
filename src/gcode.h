#ifndef GCODE_HPP
#define GCODE_HPP
#include <Arduino.h>


class GCode {
    public:
        enum Command {
            G1, G28, M999, UNDEFINED
        };
    private:
        String raw_command = "";
        Command command = UNDEFINED;
        float x_target = 0.0;
        float y_target = 0.0;
        float speed_target = 75.0; // Default 
        int previous_speed = 75.0;
        // Validity Flags
        bool valid_command = false;
        bool has_x = false;
        bool has_y = false;
        bool has_speed = false;
    public:
        GCode(String &rc);
        // Parser Functions
        void tokeniseInput(const String &rc);
        bool parseValue(const String &token); // Function also checks if input is valid, returns true if valid, false if not
        void printErrorCommand() const; // Reports to user if command is valid or not
        // Getters
        Command getCommand() const;
        float getXTarget() const;
        float getYTarget() const;
        float getSpeedTarget() const;
        bool isValid() const;


};

#endif //GCODE_HPP