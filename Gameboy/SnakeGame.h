#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

extern Adafruit_ST7735 tft;

// Function declarations
void setupSnakeGame();
bool loopSnakeGame();

#endif
