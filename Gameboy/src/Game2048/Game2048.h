#ifndef GAME2048_H
#define GAME2048_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "definitions.h"

class Game2048 {
  private:
    Adafruit_ST7735 tft;
    int board[4][4];
    bool moveInProgress;
    
    // Display settings
    static const int BOARD_OFFSET = 4;
    
    // Helper functions
    int newTileValue();
    int getRandomPoint();
    uint16_t getTileColor(int tileValue);
    uint16_t getTextColor(int tileValue);
    bool isValidPosition(int xpos, int ypos);
    
    // Game logic functions
    int moveOnce(int x, int y, bool h, bool v, bool hv, bool vv, bool noMerge);
    bool up();
    bool down();
    bool left();
    bool right();
    
    // Display functions
    void drawTile(int x, int y, int value);
    void showBoard();
    void displayGameOver();
    
    // Game state functions
    int gameStatus();
    void placeNewTile();
    void resetGame();
    
    // Joystick handling
    void handleJoystick();
    
  public:
    Game2048(int cs, int dc, int rst);
    void begin();
    bool update();
};

#endif