/*---------------------------------------------------------------------------^/
 | Names: Dillon Allan and Amir Hansen                                       |
 | ID: 0000000 and 0000001                                                   |
 | CMPUT 275, Winter 2019                                                    | 
 | Final Project: Pac Man on Arduino                                         |
/^---------------------------------------------------------------------------*/
/* Shared Objects & Global Variables */
// WARNING: PDQ library must only be included ONCE in your project to avoid
// linker errors! That's why all our PDQ-related stuff is defined here, despite
// this being a header file. Not ideal, but it was necessary in this case.
#ifndef _SHARED_H_
#define _SHARED_H_

#include <Arduino.h>
#include <SPI.h>				            // must include this here (or else IDE can't find it)
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>		        // AF: Hardware-specific library
#include "cleanMap.h"
#include "gameConfig.h"
#include "coordinates.h"
#include "global.h"
#include "mapData.h"

/// struct and class definitions

// contains properties of tft screen
struct Display {
  static const int16_t width = 128;   // ST7735 width (square display)
  static const int16_t height = 128;  // ST7735 height (square display)
  static const int16_t padding = 4;   // Reduced for smaller screen
  static const int16_t bgColor = ST7735_BLACK;

  static void drawBackground(Adafruit_ST7735 * tft) {
      tft->fillScreen(bgColor);
  }
};

// contains properties for info bars at top and bottom parts of screen
struct InfoBarData {
  static const int16_t bgColor = ST7735_BLACK;
  static const int16_t fontColor = ST7735_WHITE;
  static const char * scoreLabel;
  static const char * livesLabel;
  static const char * pauseLabel;

  // top left corner of screen
  static const Coordinates topBarLabelPos;
  static const Coordinates topBarValuePos;
  static const Coordinates topBarPausePos;
  
  // bottom left corner of screen
  static const Coordinates bottomBarLabelPos;
  static const Coordinates bottomBarValuePos;
};

// contains properties of game map
struct DrawMap {
  static const int8_t mapWidth = X_BOUND; // map width in tiles
  static const int8_t mapHeight = Y_BOUND; // map height in tiles

  static const int8_t tileSize = SCALE; // size in pixels
  static const int8_t dotSize = SCALE/4; // 1/4 of tile size
  static const int8_t pelletSize = SCALE/2; // 1/2 tile size

  static const int8_t dotOffset = (tileSize - dotSize) / 2;        // = (4-1)/2 = 1 (rounding down)
  static const int8_t pelletXOffset = (tileSize - pelletSize) / 2; // = (4-2)/2 = 1
  static const int8_t pelletYOffset = (tileSize - pelletSize) / 2; // = (4-2)/2 = 1
                                        
  static const int16_t bgColor = ST7725_DARKBLUE; // color of walls
  static const int16_t pathColor = ST7735_BLACK; // color of paths, etc.
  static const int16_t dotColor = ST7735_WHITE; // color of dot pickups
  static const int16_t pelletColor = ST7735_WHITE; // color of power pellets
  
  // top left corner of the map (in pixels) - CENTERED
  static const int16_t mapStartX = (Display::width - (mapWidth * tileSize)) / 2;
  static const int16_t mapStartY = FONT_HEIGHT + 4 + ((Display::height - FONT_HEIGHT*2 - (mapHeight * tileSize)) / 2);

  // draw map foreground to tft screen
  static void drawMap(Adafruit_ST7735 * tft);

  // draw path or non-playable area (same color)
  static void drawPath(Adafruit_ST7735 * tft, uint16_t x, uint16_t y);
  
  // draw dot (small pickup) on tft screen
  static void drawDot(Adafruit_ST7735 * tft, uint16_t x, uint16_t y);
  
  // draw power pellet (large pickup) on tft screen
  static void drawPowerPellet(Adafruit_ST7735 * tft, uint16_t x, uint16_t y);

  // draw white "door" on ghost box
  static void drawGhostDoor(Adafruit_ST7735 * tft, uint16_t x, uint16_t y);

  static void drawTile(Adafruit_ST7735 * tft, int8_t r, int8_t c);
};

// contains properties for ghosts
struct GhostData {
  // start Red on top of ghost house
  static const Coordinates redInitialPos;
  // the other ghosts are in the ghost house initially
  static const Coordinates blueInitialPos;
  static const Coordinates pinkInitialPos;
  static const Coordinates orangeInitialPos; 

  // ghost colors
  static const int16_t redColor = ST7735_RED;
  static const int16_t blueColor = ST7735_CYAN;
  static const int16_t pinkColor = ST7735_PINK;
  static const int16_t orangeColor = ST7735_ORANGE;
  static const int16_t panickedColor = ST7735_LIGHTBLUE;

  // ghost house stuff
  static const int16_t ghostDoorColor = ST7735_WHITE;
  static const int8_t  ghostDoorWidth = 2; // width in tiles
  static const int8_t  ghostDoorCol = 14; // 0-based tile position
  static const int8_t  ghostDoorRow = 16; // 0-based tile position
};

// base class for shapes that move in-game (ghosts and pac-man)
class Shape {
  public:
    // initialize in a given location with size and color
    Shape(Coordinates pos, int16_t size, int16_t color) {
      this->pos = pos;
      this->size = size;
      this->color = color;
    };

    // draw shape on touchscreen and fill previous location 
    // with display bg color
    void drawShape(Adafruit_ST7735 * tft) {
      // draw shape in current position
      tft->fillRect(lastPos.x, lastPos.y, size, size, Display::bgColor);
      tft->fillRect(pos.x, pos.y, size, size, color);
    };

    // draw shape with custom color on-screen
    void drawShape(Adafruit_ST7735 * tft, int16_t col) {
      // draw shape in current position
      tft->fillRect(lastPos.x, lastPos.y, size, size, Display::bgColor);
      tft->fillRect(pos.x, pos.y, size, size, col);
    };

    // save old position and update current position
    void setPosition(Coordinates &newPos) {
      this->lastPos = this->pos;
      this->pos = newPos;
    }

  protected:
    Coordinates pos; // in pixels
    Coordinates lastPos; // used for redrawing "trail" left behind
    // Coordinates velocity; // in pixels
    int8_t size; // all Dynamic shapes are squares of length "size"
    int16_t color;
};

// tracks shape of player character Pac-Man
class PacManShape : public Shape {
  public:
    PacManShape() : Shape(
      {11*SCALE + DrawMap::mapStartX + 1, 18*SCALE + DrawMap::mapStartY + 1}, 
      SCALE-2, 
      ST7735_YELLOW) {};
};

// tracks shape of ghosts
class GhostShape : public Shape {
  public:
    // user must provide initial position and color for a ghost
    GhostShape(Coordinates pos, int16_t color) : Shape(pos, SCALE-2,
    color) {};

    // draw panicked ghost
    void drawPanickedGhost(Adafruit_ST7735 * tft) {
      drawShape(tft, panickedColor);
    };

    // draw ghost with alternating colors to signal transition back to normal
    void drawTogglingGhost(Adafruit_ST7735 * tft) {
      if (!frightenedToggle) drawShape(tft, panickedColor);
      else drawShape(tft, color);
    }

    // toggle to draw panicked ghost
    bool frightenedToggle;

  private:
    // used when ghost is in Panicked mode
    static const int16_t panickedColor = ST7735_LIGHTBLUE;

};

// Shows current game score to user
class ScoreBar {
  public:
    // draw top bar label
    static void drawLabel(Adafruit_ST7735 * tft, Coordinates pos, 
      const char * label) {
      // set cursor position
      tft->setCursor(pos.x, pos.y);
      // draw label
      tft->println(label);
    }
    
    // draw current game score
    static void drawScore(Adafruit_ST7735 * tft, Coordinates pos, int16_t score) {
      // overwrite previous value
      tft->fillRect(pos.x, pos.y, Display::width, FONT_HEIGHT, ST7735_BLACK);

      // set cursor position and print score
      tft->setCursor(pos.x, pos.y);
      tft->print(score);
    }

    // draw status message in top bar
    static void drawPause(Adafruit_ST7735 * tft, Coordinates pos, const char * text) {
        tft->setCursor(pos.x, pos.y);
        tft->print(text);
    }
};

// Shows remaining lives to user
class LivesBar {
  public:
    // draw bottom bar label
    static void drawLabel(Adafruit_ST7735 * tft, Coordinates pos, 
      const char * label) {
      // set cursor position
      tft->setCursor(pos.x, pos.y);
      // draw label
      tft->println(label);
    }
    
    // draw current game score
    static void drawLives(Adafruit_ST7735 * tft, Coordinates pos, int16_t score) {
      // overwrite previous value
      tft->fillRect(pos.x, pos.y, Display::width, FONT_HEIGHT, ST7735_BLACK);

      // set cursor position and print score
      tft->setCursor(pos.x, pos.y);
      tft->print(score);
    }
};

// Implementations

/* static */ const char * InfoBarData::scoreLabel = "SCORE";
/* static */ const char * InfoBarData::livesLabel = "LIVES";
/* static */ const char * InfoBarData::pauseLabel = "PAUSED";

/* static */ const Coordinates InfoBarData::topBarLabelPos = 
  { Display::padding, Display::padding };
/* static */ const Coordinates InfoBarData::topBarValuePos = 
  {InfoBarData::topBarLabelPos.x + Display::width/3, InfoBarData::topBarLabelPos.y};
/* static */ const Coordinates InfoBarData::topBarPausePos = 
  {InfoBarData::topBarLabelPos.x + 3*Display::width/5, InfoBarData::topBarLabelPos.y};  

/* static */ const Coordinates InfoBarData::bottomBarLabelPos = 
  { Display::padding, Display::height - Display::padding - 2*SCALE };

/* static */ const Coordinates InfoBarData::bottomBarValuePos = 
  { InfoBarData::bottomBarLabelPos.x + Display::width/3, 
    InfoBarData::bottomBarLabelPos.y};

/* static */ const Coordinates GhostData::redInitialPos = {
  10*SCALE + DrawMap::mapStartX + 1, 10*SCALE + DrawMap::mapStartY + 1};
/* static */ const Coordinates GhostData::blueInitialPos = {
  10*SCALE + DrawMap::mapStartX + 1, 11*SCALE + DrawMap::mapStartY + 1};
/* static */ const Coordinates GhostData::pinkInitialPos = {
  13*SCALE + DrawMap::mapStartX + 1, 11*SCALE + DrawMap::mapStartY + 1};
/* static */ const Coordinates GhostData::orangeInitialPos = {
  12*SCALE + DrawMap::mapStartX + 1, 10*SCALE + DrawMap::mapStartY + 1};

/* static */ void DrawMap::drawPath(Adafruit_ST7735 * tft, uint16_t x, 
  uint16_t y) {
  tft->fillRect(x, y, tileSize, tileSize, pathColor);
}


/* static */ void DrawMap::drawDot(Adafruit_ST7735 * tft, uint16_t x, uint16_t y) {
  // fill tile first
  

  // draw dot on top of rectangle
  tft->fillRect(x + dotOffset, y + dotOffset, 
    dotSize, dotSize, dotColor);

}

/* static  */void DrawMap::drawPowerPellet(Adafruit_ST7735 * tft, uint16_t x, 
  uint16_t y) {
  // fill tile first

  // draw power pellet on top of rectangle
  tft->fillRect(x + pelletXOffset, y + pelletYOffset, 
    pelletSize, pelletSize, pelletColor);
}


/* static */ void DrawMap::drawGhostDoor(Adafruit_ST7735 * tft, uint16_t x, 
  uint16_t y) {
  tft->drawFastHLine(x, y, GhostData::ghostDoorWidth*tileSize, 
  GhostData::ghostDoorColor);
}


/* static */void DrawMap::drawTile(Adafruit_ST7735 * tft, int8_t r, 
  int8_t c) {
    switch (myMap.mapLayout[r][c]) {
        case MapData::barePath: // draw same color in both cases
        case MapData::nonPlayArea:
          drawPath(tft, mapStartX + c*tileSize, 
              mapStartY + r*tileSize);
          break;

        case MapData::dot: // draw dot on tile
          drawPath(tft, mapStartX + c*tileSize, 
              mapStartY + r*tileSize);
          drawDot(tft, mapStartX + c*tileSize, 
              mapStartY + r*tileSize);
          break;

        case MapData::powerPellet: // draw power pellet on tile 
          drawPath(tft, mapStartX + c*tileSize, 
              mapStartY + r*tileSize);
          drawPowerPellet(tft, mapStartX + c*tileSize, 
              mapStartY + r*tileSize);
          break;
      }
  }

/* static  */void DrawMap::drawMap(Adafruit_ST7735 * tft) {
  // fill background
  tft->fillRect(mapStartX, mapStartY, mapWidth*tileSize, mapHeight*tileSize, 
    bgColor);
  
  for (int8_t r = 0; r < mapHeight; ++r) {
    // left half
    for (int8_t c = 0; c < mapWidth; ++c) {
      drawTile(tft,r,c);
    }
  }

  // draw ghost door
  drawGhostDoor(tft, GhostData::ghostDoorCol*tileSize, 
    GhostData::ghostDoorRow*tileSize);
};
#endif
