#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

extern Adafruit_ST7735 tft;

// Function declarations
void setupMinesweeper();
bool loopMinesweeper();

#endif
