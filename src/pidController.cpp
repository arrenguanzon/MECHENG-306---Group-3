#include "pidController.h"
#include <Arduino.h> // not too sure if we need it 

// error = targetMotor1 - currentMotor1
// consider CASE: input targetMotor1 is negative

// if error > 0, motor goes forward (HIGH) @ motor 1 speed = Kp * error + Ki * integral
// if error < 0, motor goes backward (LOW) @ motor 1 speed = Kp * error + Ki * integral