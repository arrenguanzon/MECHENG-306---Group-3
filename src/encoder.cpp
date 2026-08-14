// #include "encoder.h"
// #include <Arduino.h> // not too sure if we need it

// volatile int currentCount = 0;

// encoder::encoder()
// {
//     currentPos = 0;
//     destinationPos = 0;
//     currentCount = 0;
// }

// int encoder::convertToCounts(int mm)
// {
//     return mm * 9.513;
// }

// int encoder::counterUp(int currentCount)
// {
//     currentCount++;
//     return currentCount;
// }

// //ARDUNIO CODE

// encoder encoderObject;

// void encoder1A();

// // Set up encoder pins
// #define ENCODER1_A 18
// #define ENCODER1_B 19
// #define ENCODER2_A 20
// #define ENCODER2_B 21

// void setup()
// {
//   pinMode(ENCODER1_A, INPUT_PULLUP);
//   pinMode(ENCODER1_B, INPUT_PULLUP);
//   pinMode(ENCODER2_A, INPUT_PULLUP);
//   pinMode(ENCODER2_B, INPUT_PULLUP);
// }
// // instants of objects ( do we need them or can we just do static functions??)


// void loop()
// {

//     //every time quadrator encoder is triggered ISR will be called and currentCount will be incremented
//     attachInterrupt(digitalPinToInterrupt(ENCODER1_A), encoder1A, CHANGE);

// }

// void encoder1A()
// {
//     // check direction of encoder
//     if (digitalRead(ENCODER1_A) == digitalRead(ENCODER1_B))
//     {
//         // moving forward
//         currentCount++;
//     }
//     else
//     {
//         // moving backward
//         currentCount--;
//     }
// }

// // interrupts
