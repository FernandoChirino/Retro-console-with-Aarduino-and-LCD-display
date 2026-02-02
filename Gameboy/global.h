/*---------------------------------------------------------------------------^/
 | Names: Dillon Allan and Amir Hansen                                       |
 | ID: 0000000 and 0000001                                                   |
 | CMPUT 275, Winter 2019                                                    | 
 | Final Project: Pac Man on Arduino                                         |
/^---------------------------------------------------------------------------*/
/*Globals used throughout other game files.*/
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "mapData.h"
#include "controller.h"
#include "game.h"
#include "pacMan.h"
#include "ghost.h"

// globals for player controls, game map, and overall game state
extern Controller con;
extern MapData myMap;
extern Game game; 

// game actors
extern PacMan pac;
extern Ghost red;
extern Ghost pink;
extern Ghost blue;
extern Ghost orange;

extern Adafruit_ST7735 tft;

#endif
