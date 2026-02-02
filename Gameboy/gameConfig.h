/*---------------------------------------------------------------------------^/
 | Names: Dillon Allan and Amir Hansen                                       |
 | ID: 0000000 and 0000001                                                   |
 | CMPUT 275, Winter 2019                                                    | 
 | Final Project: Pac Man on Arduino                                         |
/^---------------------------------------------------------------------------*/
/* Game constant configuration */
#ifndef game_config
#define game_config

#define FRAME_DELAY 30 // in milliseconds
#define FONT_SIZE 1    // CHANGED: Smaller font for 128x128 screen
#define FONT_HEIGHT 8*FONT_SIZE // in pixels (now 8 instead of 16)

// CRITICAL CHANGES FOR 128x128 DISPLAY:
#define Y_BOUND 24     // CHANGED: Reduced from 31 (fewer tiles vertically)
#define X_BOUND 24     // CHANGED: Reduced from 28 (fewer tiles horizontally)
#define SCALE 4        // CHANGED: Reduced from 8 (smaller tiles)

// ST7735 Color Definitions (replacing ILI9341 colors)
#define ST7735_PINK 0xFE19      // Pink color in RGB565
#define ST7735_LIGHTBLUE 0x867D // Light blue color in RGB565
#define ST7725_DARKBLUE 0x0032

/* For the Joystick */
#define DEAD_ZONE 64*64
#define STICK_OFFSET 512

#endif
