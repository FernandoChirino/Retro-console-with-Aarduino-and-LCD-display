#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Function declarations
void breakoutSetup();
void breakoutLoop();
bool breakoutCheckReturnToMenu();

// External references to objects and variables from main file
extern Adafruit_ST7735 tft;
extern const int joyX;
extern const int joyY;
extern const int joyButton;
extern const int button;

#endif