/*---------------------------------------------------------------------------^/
 | Names: Dillon Allan and Amir Hansen                                       |
 | ID: 0000000 and 0000001                                                   |
 | CMPUT 275, Winter 2019                                                    | 
 | Final Project: Pac Man on Arduino                                         |
/^---------------------------------------------------------------------------*/
/*Globals used throughout other game files.*/
#include "mapData.h"
#include "controller.h"
#include "game.h"
#include "pacMan.h"
#include "ghost.h"
#include <Arduino.h>
Controller con;
MapData myMap;
Game game(3); // 3 lives...
PacMan pac;
// NEW positions (for 24x24 map)
Ghost red(10.0f,11.0f,0.1f,DOWN,0);        // row 10, col 11
Ghost pink(11.0f,13.0f,0.125f,UP,300);     // row 11, col 13
Ghost blue(11.0f,10.0f,0.125f,RIGHT,200);  // row 11, col 10
Ghost orange(10.0f,12.0f,0.125f,LEFT,100); // row 10, col 12
