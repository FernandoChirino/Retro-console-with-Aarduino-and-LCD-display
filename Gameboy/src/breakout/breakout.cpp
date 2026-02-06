#include "breakout.h"
#include <Arduino.h>

// Forward declarations
void newGame(int levelIndex);
void setupState();
void setupWall();
void drawBrick(int xBrick, int yBrickRow, uint16_t color);
void breakoutDrawPlayer();
void drawBall(int x, int y, int xold, int yold, int ballsize);
void updateLives(int lives, int remainingLives);
void updateScore(int score);
void checkBrickCollision(uint16_t x, uint16_t y);
int checkCornerCollision(uint16_t x, uint16_t y);
void hitBrick(int xBrick, int yBrickRow);
void checkBorderCollision(uint16_t x, uint16_t y);
void checkBallCollisions(uint16_t x, uint16_t y);
void checkBallExit(uint16_t x, uint16_t y);
void readJoystickInput();
void waitForButton();
void gameOverWaitForButton();
void clearDialog();
boolean noBricks();
void setBrick(int wall[], uint8_t x, uint8_t y);
void unsetBrick(int wall[], uint8_t x, uint8_t y);
boolean isBrickIn(int wall[], uint8_t x, uint8_t y);

// Color definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define SCORE_SIZE 12

const uint8_t BIT_MASK[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t pointsForRow[] = {7, 7, 5, 5, 3, 3, 1, 1};

#define GAMES_NUMBER 5

typedef struct game_type {
  int ballsize;
  int playerwidth;
  int playerheight;
  int exponent;
  int top;
  int rows;
  int columns;
  int brickGap;
  int lives;
  int wall[8];
  int initVelx;
  int initVely;
} game_type;

game_type games[GAMES_NUMBER] = {
  // ballsize, playerwidth, playerheight, exponent, top, rows, columns, brickGap, lives, wall[8], initVelx, initVely
  {4,  20, 3, 4, 15, 4, 6, 1, 3, {0xFF, 0xDB, 0xDB, 0xFF, 0x00, 0x00, 0x00, 0x00}, 12, -12},
  {4,  18, 3, 4, 15, 5, 6, 1, 3, {0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0x00, 0x00, 0x00}, 14, -14},
  {4,  16, 3, 4, 15, 6, 7, 1, 3, {0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0x00, 0x00}, 16, -16},
  {3,  15, 3, 4, 15, 6, 8, 1, 3, {0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0x00, 0x00}, 18, -18},
  {3,  14, 3, 4, 15, 8, 8, 1, 3, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 20, -20}
};

typedef struct game_state_type {
  uint16_t ballx;
  uint16_t bally;
  uint16_t ballxold;
  uint16_t ballyold;
  int velx;
  int vely;
  int playerx;
  int playerxold;
  int wallState[8];
  int score;
  int remainingLives;
  int bottom;
  int walltop;
  int wallbottom;
  int brickheight;
  int brickwidth;
} game_state_type;

// Game variables
game_type* currentGame;
game_state_type breakoutState;
int breakoutLevel = 0;
uint16_t backgroundColor = BLACK;
boolean breakoutGameOver = false;
boolean breakoutReturnToMenu = false;

// Joystick variables
const int JOY_THRESHOLD = 200;
const int JOY_CENTER = 512;
const int PLAYER_SPEED = 2;

char scoreFormat[] = "%04d";

void setupState() {
  breakoutState.bottom = tft.height() - 8;
  breakoutState.brickwidth = tft.width() / currentGame->columns;
  breakoutState.brickheight = (tft.height() - SCORE_SIZE - 20) / 12;
  
  for (int i = 0; i < currentGame->rows; i++) {
    breakoutState.wallState[i] = 0;
  }
  
  breakoutState.playerx = tft.width() / 2 - currentGame->playerwidth / 2;
  breakoutState.playerxold = breakoutState.playerx;
  breakoutState.remainingLives = currentGame->lives;
  breakoutState.bally = breakoutState.bottom << currentGame->exponent;
  breakoutState.ballyold = breakoutState.bottom << currentGame->exponent;
  breakoutState.velx = currentGame->initVelx;
  breakoutState.vely = currentGame->initVely;
  breakoutState.ballx = (tft.width() / 2) << currentGame->exponent;
  breakoutState.ballxold = breakoutState.ballx;
}

void updateLives(int lives, int remainingLives) {
  for (int i = 0; i < lives; i++) {
    tft.fillCircle((1 + i) * 8, 5, 2, BLACK);
  }
  
  for (int i = 0; i < remainingLives; i++) {
    tft.fillCircle((1 + i) * 8, 5, 2, YELLOW);
  }
}

void setupWall() {
  int colors[] = {RED, RED, BLUE, BLUE, YELLOW, YELLOW, GREEN, GREEN};
  breakoutState.walltop = currentGame->top + SCORE_SIZE;
  breakoutState.wallbottom = breakoutState.walltop + currentGame->rows * breakoutState.brickheight;
  
  for (int i = 0; i < currentGame->rows; i++) {
    for (int j = 0; j < currentGame->columns; j++) {
      if (isBrickIn(currentGame->wall, j, i)) {
        setBrick(breakoutState.wallState, j, i);
        drawBrick(j, i, colors[i]);
      }
    }
  }
}

void drawBrick(int xBrick, int yBrickRow, uint16_t color) {
  tft.fillRect((breakoutState.brickwidth * xBrick) + currentGame->brickGap,
               breakoutState.walltop + (breakoutState.brickheight * yBrickRow) + currentGame->brickGap,
               breakoutState.brickwidth - currentGame->brickGap * 2,
               breakoutState.brickheight - currentGame->brickGap * 2, color);
}

boolean noBricks() {
  for (int i = 0; i < currentGame->rows; i++) {
    if (breakoutState.wallState[i]) return false;
  }
  return true;
}

void breakoutDrawPlayer() {
  tft.fillRect(breakoutState.playerx, breakoutState.bottom, 
               currentGame->playerwidth, currentGame->playerheight, YELLOW);
  
  if (breakoutState.playerx != breakoutState.playerxold) {
    if (breakoutState.playerx < breakoutState.playerxold) {
      tft.fillRect(breakoutState.playerx + currentGame->playerwidth, breakoutState.bottom,
                   abs(breakoutState.playerx - breakoutState.playerxold), 
                   currentGame->playerheight, backgroundColor);
    } else {
      tft.fillRect(breakoutState.playerxold, breakoutState.bottom,
                   abs(breakoutState.playerx - breakoutState.playerxold), 
                   currentGame->playerheight, backgroundColor);
    }
  }
}

void drawBall(int x, int y, int xold, int yold, int ballsize) {
  if (xold <= x && yold <= y) {
    tft.fillRect(xold, yold, ballsize, y - yold, BLACK);
    tft.fillRect(xold, yold, x - xold, ballsize, BLACK);
  } else if (xold >= x && yold >= y) {
    tft.fillRect(x + ballsize, yold, xold - x, ballsize, BLACK);
    tft.fillRect(xold, y + ballsize, ballsize, yold - y, BLACK);
  } else if (xold <= x && yold >= y) {
    tft.fillRect(xold, yold, x - xold, ballsize, BLACK);
    tft.fillRect(xold, y + ballsize, ballsize, yold - y, BLACK);
  } else if (xold >= x && yold <= y) {
    tft.fillRect(xold, yold, ballsize, y - yold, BLACK);
    tft.fillRect(x + ballsize, yold, xold - x, ballsize, BLACK);
  }
  
  tft.fillRect(x, y, ballsize, ballsize, YELLOW);
}

void waitForButton() {
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.setCursor(40, 70);
  tft.print("BREAKOUT");
  tft.setCursor(20, 85);
  tft.print("Press button");
  
  // Wait for button press (not joystick button)
  while (digitalRead(button) == HIGH) {
    delay(10);
  }
  delay(300);
}

void gameOverWaitForButton() {
  // Flash effect
  for (int i = 0; i < 3; i++) {
    tft.invertDisplay(true);
    delay(100);
    tft.invertDisplay(false);
    delay(100);
  }
  
  tft.fillRect(15, 40, 100, 70, ST7735_BLACK);
  tft.drawRect(15, 40, 100, 70, ST7735_RED);
  tft.drawRect(16, 41, 98, 68, ST7735_RED);
  
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.setCursor(37, 48);
  tft.print("GAME");
  tft.setCursor(37, 64);
  tft.print("OVER");
  
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(16, 85);
  tft.print("Press to restart");
  tft.setCursor(16, 95);
  tft.print("Joy btn for menu");
  
  // Wait for either button (restart) or joystick button (menu)
  while (true) {
    if (digitalRead(button) == LOW) {
      delay(300);
      breakoutReturnToMenu = false;
      return;
    }
    if (digitalRead(joyButton) == LOW) {
      delay(300);
      breakoutReturnToMenu = true;
      return;
    }
    delay(10);
  }
}

void updateScore(int score) {
  char buffer[5];
  snprintf(buffer, sizeof(buffer), scoreFormat, score);
  tft.fillRect(tft.width() - 30, 2, 28, 8, BLACK);
  tft.setTextColor(YELLOW);
  tft.setTextSize(1);
  tft.setCursor(tft.width() - 28, 2);
  tft.print(buffer);
}

void checkBrickCollision(uint16_t x, uint16_t y) {
  int x1 = x + currentGame->ballsize;
  int y1 = y + currentGame->ballsize;
  int collisions = 0;
  
  collisions += checkCornerCollision(x, y);
  collisions += checkCornerCollision(x1, y1);
  collisions += checkCornerCollision(x, y1);
  collisions += checkCornerCollision(x1, y);
  
  if (collisions > 0) {
    breakoutState.vely = (-1 * breakoutState.vely);
    if ((((x % breakoutState.brickwidth) == 0) && (breakoutState.velx < 0)) ||
        ((((x + currentGame->ballsize) % breakoutState.brickwidth) == 0) && 
         (breakoutState.velx > 0))) {
      breakoutState.velx = (-1 * breakoutState.velx);
    }
  }
}

int checkCornerCollision(uint16_t x, uint16_t y) {
  if ((y > breakoutState.walltop) && (y < breakoutState.wallbottom)) {
    int yBrickRow = (y - breakoutState.walltop) / breakoutState.brickheight;
    int xBrickColumn = (x / breakoutState.brickwidth);
    
    if (isBrickIn(breakoutState.wallState, xBrickColumn, yBrickRow)) {
      hitBrick(xBrickColumn, yBrickRow);
      return 1;
    }
  }
  return 0;
}

void hitBrick(int xBrick, int yBrickRow) {
  breakoutState.score += pointsForRow[yBrickRow];
  drawBrick(xBrick, yBrickRow, WHITE);
  delay(16);
  drawBrick(xBrick, yBrickRow, BLUE);
  delay(8);
  drawBrick(xBrick, yBrickRow, backgroundColor);
  unsetBrick(breakoutState.wallState, xBrick, yBrickRow);
  updateScore(breakoutState.score);
}

void checkBorderCollision(uint16_t x, uint16_t y) {
  if (x + currentGame->ballsize >= tft.width()) {
    breakoutState.velx = -abs(breakoutState.velx);
  }
  if (x <= 0) {
    breakoutState.velx = abs(breakoutState.velx);
  }
  if (y <= SCORE_SIZE) {
    breakoutState.vely = abs(breakoutState.vely);
  }
  if (((y + currentGame->ballsize) >= breakoutState.bottom) &&
      ((y + currentGame->ballsize) <= (breakoutState.bottom + currentGame->playerheight)) &&
      (x >= breakoutState.playerx) &&
      (x <= (breakoutState.playerx + currentGame->playerwidth))) {
    
    if (x > (breakoutState.playerx + currentGame->playerwidth - 3)) {
      breakoutState.velx = breakoutState.velx - 1;
    } else if (x < breakoutState.playerx + 3) {
      breakoutState.velx = breakoutState.velx + 1;
    }
    breakoutState.vely = -abs(breakoutState.vely);
  }
}

void checkBallCollisions(uint16_t x, uint16_t y) {
  checkBrickCollision(x, y);
  checkBorderCollision(x, y);
}

void checkBallExit(uint16_t x, uint16_t y) {
  if ((y + currentGame->ballsize) >= tft.height()) {
    breakoutState.remainingLives--;
    updateLives(currentGame->lives, breakoutState.remainingLives);
    delay(500);
    breakoutState.vely = -abs(breakoutState.vely);
    breakoutState.ballx = (tft.width() / 2) << currentGame->exponent;
    breakoutState.bally = breakoutState.bottom << currentGame->exponent;
  }
}

void setBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] | BIT_MASK[x];
}

void unsetBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] & ~BIT_MASK[x];
}

boolean isBrickIn(int wall[], uint8_t x, uint8_t y) {
  return wall[y] & BIT_MASK[x];
}

void clearDialog() {
  tft.fillScreen(backgroundColor);
  tft.fillRect(0, 0, tft.width(), SCORE_SIZE, BLACK);
}

void readJoystickInput() {
  int joyXVal = analogRead(joyX);
  
  if (joyXVal > JOY_CENTER + JOY_THRESHOLD) {
    breakoutState.playerx -= PLAYER_SPEED;
  } else if (joyXVal < JOY_CENTER - JOY_THRESHOLD) {
    breakoutState.playerx += PLAYER_SPEED;
  }
  
  if (breakoutState.playerx >= tft.width() - currentGame->playerwidth) {
    breakoutState.playerx = tft.width() - currentGame->playerwidth;
  }
  if (breakoutState.playerx < 0) {
    breakoutState.playerx = 0;
  }
}

void newGame(int levelIndex) {
  currentGame = &games[levelIndex];
  setupState();
  
  clearDialog();
  updateLives(currentGame->lives, breakoutState.remainingLives);
  updateScore(breakoutState.score);
  setupWall();
  
  waitForButton();
  
  clearDialog();
  updateLives(currentGame->lives, breakoutState.remainingLives);
  updateScore(breakoutState.score);
  setupWall();
}

void breakoutSetup() {
  tft.fillScreen(BLACK);
  
  tft.setTextColor(CYAN);
  tft.setTextSize(2);
  tft.setCursor(40, 50);
  tft.print("BREAK");
  tft.setCursor(40, 68);
  tft.print("OUT!");
  delay(1500);
  
  breakoutLevel = 0;
  breakoutState.score = 0;
  breakoutGameOver = false;
  breakoutReturnToMenu = false;
  
  newGame(breakoutLevel);
}

void breakoutLoop() {
  if (breakoutReturnToMenu) {
    return;
  }
  
  // Check if joystick button is pressed to return to menu during gameplay
  if (digitalRead(joyButton) == LOW) {
    delay(300);
    breakoutReturnToMenu = true;
    return;
  }
  
  readJoystickInput();
  
  breakoutDrawPlayer();
  breakoutState.playerxold = breakoutState.playerx;
  
  // Check max speed
  if (abs(breakoutState.vely) > ((1 << currentGame->exponent) - 1)) {
    breakoutState.vely = ((1 << currentGame->exponent) - 1) * 
                         ((breakoutState.vely > 0) - (breakoutState.vely < 0));
  }
  if (abs(breakoutState.velx) > ((1 << currentGame->exponent) - 1)) {
    breakoutState.velx = ((1 << currentGame->exponent) - 1) * 
                         ((breakoutState.velx > 0) - (breakoutState.velx < 0));
  }
  
  breakoutState.ballx += breakoutState.velx;
  breakoutState.bally += breakoutState.vely;
  
  checkBallCollisions(breakoutState.ballx >> currentGame->exponent, 
                     breakoutState.bally >> currentGame->exponent);
  checkBallExit(breakoutState.ballx >> currentGame->exponent, 
               breakoutState.bally >> currentGame->exponent);
  
  drawBall(breakoutState.ballx >> currentGame->exponent, 
           breakoutState.bally >> currentGame->exponent,
           breakoutState.ballxold >> currentGame->exponent, 
           breakoutState.ballyold >> currentGame->exponent, 
           currentGame->ballsize);
  
  breakoutState.ballxold = breakoutState.ballx;
  breakoutState.ballyold = breakoutState.bally;
  
  breakoutState.velx = (12 + (breakoutState.score >> 4)) * 
                       ((breakoutState.velx > 0) - (breakoutState.velx < 0));
  breakoutState.vely = (12 + (breakoutState.score >> 4)) * 
                       ((breakoutState.vely > 0) - (breakoutState.vely < 0));
  
  if (noBricks() && breakoutLevel < GAMES_NUMBER - 1) {
    breakoutLevel++;
    newGame(breakoutLevel);
  } else if (breakoutState.remainingLives <= 0) {
    gameOverWaitForButton();
    if (!breakoutReturnToMenu) {
      breakoutState.score = 0;
      breakoutLevel = 0;
      newGame(breakoutLevel);
    }
  }
  
  delay(10);
}

bool breakoutCheckReturnToMenu() {
  return breakoutReturnToMenu;
}