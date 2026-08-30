#ifndef GCODE_HPP
#define GCODE_HPP
#include <Arduino.h>

class GCode {
    public:
        enum Command {
            G1, // Linear movement with X, Y, F parameters
            G28, // Homing command
            M999, // Reset/fault recovery
            UNDEFINED // Invalid or unrecognized command
        };
    private:
        String raw_command = ""; // Original input string
        Command command = UNDEFINED; // Parsed command type
        float x_target = 0.0; // Target X displacement (mm)
        float y_target = 0.0; // Target Y displacement (mm)
        float speed_target = 150.0; // Target movement speed (PWM value)
        float previous_speed = 150.0; // Previous speed for inheritance
        
        bool valid_command = false; // Command validation status
        bool has_x = false; // Flag: X parameter present
        bool has_y = false; // Flag: Y parameter present
        bool has_speed = false; // Flag: F (speed) parameter present
        
        float& absoluteX; // Reference to absolute X position
        float& absoluteY; // Reference to absolute Y position
    public:
        // Parse a G-code string from raw input

        // abs_x and abs_y are references to current absolute position
        GCode(String &rc, float& abs_x, float& abs_y);
        
        // Convert feedrate (mm/min) to PWM value (0-255)
        float FeedrateToPWM(float feedrate);
        
        // Tokenize and parse input string into command and parameters
        void tokeniseInput(const String &rc);
        
        // Parse a single token (X, Y, or F parameter), returns true if token is valid, false otherwise
        bool parseValue(const String &token);
        
        // Print error message for invalid command to serial
        void printErrorCommand() const;
        
        // Check that target position is within gantry bounds
        bool checkBounds();
        
        // Get parsed command type
        Command getCommand() const;
        
        // Get X target displacement (mm)
        float getXTarget() const;
        
        // Get Y target displacement (mm)
        float getYTarget() const;
        
        // Get target movement speed (PWM value)
        float getSpeedTarget() const;
        
        // Check if command is valid
        bool isValid() const;




};








#endif //GCODE_HPP