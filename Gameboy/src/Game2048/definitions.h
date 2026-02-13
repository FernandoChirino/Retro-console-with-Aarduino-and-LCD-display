// This file contains definitions to be used throughout the 2048 project.
#ifndef DEFINITIONS
  #define DEFINITIONS
  
  // ST7735 Color definitions (16-bit RGB565 format)
  #define BLACK   0x0000
  #define BLUE    0x001F
  #define RED     0xF800
  #define GREEN   0x07E0
  #define CYAN    0x07FF
  #define MAGENTA 0xF81F
  #define YELLOW  0xFFE0
  #define WHITE   0xFFFF
  #define ORANGE  0xFD20
  #define PURPLE  0x780F
  #define BROWN   0x8200
  #define GREY    0x7BEF
  #define DARKGREY 0x4208
  #define LIGHTGREY 0xBDF7
  
  // Additional colors for 2048 tiles
  #define TILE_2     0xEF7D  // Light beige
  #define TILE_4     0xEE59  // Beige
  #define TILE_8     0xFCA8  // Orange
  #define TILE_16    0xFB23  // Dark orange
  #define TILE_32    0xFB00  // Red-orange
  #define TILE_64    0xF900  // Red
  #define TILE_128   0xFE60  // Yellow
  #define TILE_256   0xFE40  // Dark yellow
  #define TILE_512   0xFE20  // Darker yellow
  #define TILE_1024  0xFE00  // Gold
  #define TILE_2048  0xFDA0  // Golden orange
  #define TILE_EMPTY 0x9CF3  // Light grey
  
  // Joystick pins
  #define JOY_X A5
  #define JOY_Y A4
  #define JOY_BTN 4  // Optional select/reset button
  
  // Joystick threshold values (adjust these based on your joystick)
  #define JOY_CENTER 512
  #define JOY_DEADZONE 100
  
  // Display settings
  #define SCREEN_SIZE 128
  #define TILE_SIZE 30
  #define TILE_MARGIN 2
  #define BOARD_SIZE 4  // 4x4 game board
  
#endif
