#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H

#include "config.h"

void tokeniseInput(String gcode_string, String tokens[]);
void extractParameters(String tokens[], MovementType &movementType, float &xOffset, float &yOffset, float &motionSpeed);
void trackPosition(float &currentXPosition, float &currentYPosition, float &xOffset, float &yOffset, float &newXPosition, float &newYPosition);
bool validateMove(MovementType &movementType, float &motionSpeed, float &xOffset, float &yOffset, float &newXPosition, float &newYPosition);

#endif
