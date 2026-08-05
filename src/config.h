#ifndef CONFIG_H
#define CONFIG_H

#define motor1_pin 4
#define enable1_pin 5
#define enable2_pin 6
#define motor2_pin 7

#define switch_top 16
#define switch_bottom 17
#define switch_left 18
#define switch_right 19

// #define home_speed 100

// enum SwitchState {sT, sB, sL, sR, START};
enum States {IDLE, HOMING, MOVING, FAULT};
enum MovementType {G01, G28, M999};

#endif
