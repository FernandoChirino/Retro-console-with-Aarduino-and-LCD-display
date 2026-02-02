#ifndef DODGE_H
#define DODGE_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Function declarations
void dodgeSetup();
void dodgeLoop();
bool dodgeCheckReturnToMenu();

// External references to objects and variables from main file
extern Adafruit_ST7735 tft;
extern const int joyX;
extern const int joyY;
extern const int joyButton;
extern const int button;

#endif