#include "tetris.h"
#include <Arduino.h>

// Forward declarations (declare all functions before using them)
void checkLines();
void breakLine(short line);
void drawGrid();
void erasePiece(short oldPiece[2][4], short oldX, short oldY);
void drawPiece(short type, short x, short y);
boolean nextHorizontalCollision(short testPiece[2][4], int amount);
boolean nextCollision();
boolean checkGameOver();
void showGameOver();
void resetGame();
void generate();
void drawNextPiece();
void copyPiece(short piece[2][4], short type, short rotation);
short getMaxRotation(short type);
boolean canRotate(short rotation);
void updateScore();
void drawLayout();
void updatePiece();

// Tetris piece definitions: [rotations][x,y coordinates][4 blocks per piece]
const char pieces_S_l[2][2][4] = {{  // S-piece (left orientation)
                                    {0, 0, 1, 1}, {0, 1, 1, 2}
                                  },
                                  {
                                    {0, 1, 1, 2}, {1, 1, 0, 0}
                                  }};

const char pieces_S_r[2][2][4] = {{  // S-piece (right orientation)
                                    {1, 1, 0, 0}, {0, 1, 1, 2}
                                  },
                                  {
                                    {0, 1, 1, 2}, {0, 0, 1, 1}
                                  }};

const char pieces_L_l[4][2][4] = {{  // L-piece (left): 4 rotation states
                                    {0, 0, 0, 1}, {0, 1, 2, 2}
                                  },
                                  {
                                    {0, 1, 2, 2}, {1, 1, 1, 0}
                                  },
                                  {
                                    {0, 1, 1, 1}, {0, 0, 1, 2}
                                  },
                                  {
                                    {0, 0, 1, 2}, {1, 0, 0, 0}
                                  }};

const char pieces_Sq[1][2][4] = {{  // Square: no rotation needed
                                    {0, 1, 0, 1}, {0, 0, 1, 1}
                                  }};

const char pieces_T[4][2][4] = {{   // T-piece: 4 rotation states
                                    {0, 0, 1, 0}, {0, 1, 1, 2}
                                  },
                                  {
                                    {0, 1, 1, 2}, {1, 0, 1, 1}
                                  },
                                  {
                                    {1, 0, 1, 1}, {0, 1, 1, 2}
                                  },
                                  {
                                    {0, 1, 1, 2}, {0, 0, 1, 0}
                                  }};

const char pieces_l[2][2][4] = {{   // I-piece (line): 2 rotation states
                                    {0, 1, 2, 3}, {0, 0, 0, 0}
                                  },
                                  {
                                    {0, 0, 0, 0}, {0, 1, 2, 3}
                                  }};

// Game constants
const short MARGIN_TOP = 20;   // Game grid starting Y position
const short MARGIN_LEFT = 30;  // Game grid starting X position
const short SIZE = 5;          // Block size in pixels
const short TYPES = 6;         // Number of piece types

// Color definitions
const uint16_t PIECE_COLORS[6] = {
  ST7735_CYAN,    // Type 0: L-piece
  ST7735_GREEN,   // Type 1: S-piece left
  ST7735_RED,     // Type 2: S-piece right
  ST7735_YELLOW,  // Type 3: Square
  ST7735_MAGENTA, // Type 4: T-piece
  ST7735_BLUE     // Type 5: I-piece
};

// Game state variables
word currentType, nextType, rotation;
short pieceX, pieceY;                 // Current falling piece position
short oldPieceX, oldPieceY;           // Previous frame piece position
short piece[2][4];                    // Current piece coordinates
short oldPiece[2][4];                 // Previous frame piece coordinates
int interval = 400, score;            // Fall speed, player score
int oldScore = 0;                     // Score from last display update
long timer, delayer;                  // Game loop timer, input delay timer
boolean grid[10][18];                 // Game board: occupied cells (10 wide, 18 tall)
uint16_t gridColors[10][18];          // Color of each placed block
boolean b1, b2, b3;                   // Input state flags
boolean gameOver = false;             // Game over state
boolean returnToMenu = false;         // Flag to return to main menu

// Joystick debounce and sensitivity
unsigned long lastMoveTime = 0; // Last time a piece was moved
const int MOVE_DELAY = 150;     // Minimum time between moves (ms)
const int JOY_THRESHOLD = 200;  // Joystick deadzone (away from center 512)

void checkLines() {
  boolean full;
  for (short y = 17; y >= 0; y--) {
    full = true;
    for (short x = 0; x < 10; x++) {
      full = full && grid[x][y];  // Check if entire row is filled
    }
    if (full) {
      breakLine(y);
      y++;  // Recheck this line as blocks fall
    }
  }
}

void breakLine(short line) {
  tft.invertDisplay(true);  // Flash effect
  delay(50);
  tft.invertDisplay(false);
  
  // Shift all lines above down by one
  for (short y = line; y >= 0; y--) {
    for (short x = 0; x < 10; x++) {
      grid[x][y] = grid[x][y - 1];
      gridColors[x][y] = gridColors[x][y - 1];
    }
  }
  
  // Clear top line
  for (short x = 0; x < 10; x++) {
    grid[x][0] = 0;
  }
  
  score += 10;
  drawGrid();
}

void drawGrid() {
  tft.fillRect(MARGIN_LEFT, MARGIN_TOP, 61, 109, ST7735_BLACK);
  
  for (short x = 0; x < 10; x++) {
    for (short y = 0; y < 18; y++) {
      if (grid[x][y]) {
        tft.fillRect(MARGIN_LEFT + (SIZE + 1) * x, 
                     MARGIN_TOP + (SIZE + 1) * y, 
                     SIZE, SIZE, gridColors[x][y]);
      }
    }
  }
}

void erasePiece(short oldPiece[2][4], short oldX, short oldY) {
  for (short i = 0; i < 4; i++) {
    short x = oldX + oldPiece[0][i];
    short y = oldY + oldPiece[1][i];
    
    if (!grid[x][y]) {
      tft.fillRect(MARGIN_LEFT + (SIZE + 1) * x, 
                   MARGIN_TOP + (SIZE + 1) * y, 
                   SIZE, SIZE, ST7735_BLACK);
    } else {
      tft.fillRect(MARGIN_LEFT + (SIZE + 1) * x, 
                   MARGIN_TOP + (SIZE + 1) * y, 
                   SIZE, SIZE, gridColors[x][y]);
    }
  }
}

void drawPiece(short type, short x, short y) {
  for (short i = 0; i < 4; i++) {
    tft.fillRect(MARGIN_LEFT + (SIZE + 1) * (x + piece[0][i]), 
                 MARGIN_TOP + (SIZE + 1) * (y + piece[1][i]), 
                 SIZE, SIZE, PIECE_COLORS[type]);
  }
}

boolean nextHorizontalCollision(short testPiece[2][4], int amount) {
  for (short i = 0; i < 4; i++) {
    short newX = pieceX + testPiece[0][i] + amount;
    short newY = pieceY + testPiece[1][i];
    // Check boundaries and existing blocks
    if (newX > 9 || newX < 0 || grid[newX][newY])
      return true;
  }
  return false;
}

boolean nextCollision() {
  for (short i = 0; i < 4; i++) {
    short y = pieceY + piece[1][i] + 1;
    short x = pieceX + piece[0][i];
    // Check bottom boundary or occupied cell
    if (y > 17 || grid[x][y])
      return true;
  }
  return false;
}

boolean checkGameOver() {
  // Check if blocks reached top rows
  for (short x = 0; x < 10; x++) {
    if (grid[x][0] || grid[x][1]) {
      return true;
    }
  }
  return false;
}

void showGameOver() {
  for (int i = 0; i < 3; i++) {
    tft.invertDisplay(true);
    delay(100);
    tft.invertDisplay(false);
    delay(100);
  }
  
  tft.fillRect(15, 40, 98, 70, ST7735_BLACK);
  tft.drawRect(15, 40, 98, 70, ST7735_RED);
  tft.drawRect(16, 41, 96, 68, ST7735_RED);
  
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.setCursor(25, 48);
  tft.print("GAME");
  tft.setCursor(25, 64);
  tft.print("OVER");
  
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 85);
  tft.print("Score: ");
  tft.print(score);
  
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(20, 100);
  tft.print("Press to menu");
  
  gameOver = true;
}

void resetGame() {
  for (short x = 0; x < 10; x++) {
    for (short y = 0; y < 18; y++) {
      grid[x][y] = 0;
      gridColors[x][y] = ST7735_BLACK;
    }
  }
  
  score = 0;
  oldScore = 0;
  gameOver = false;
  returnToMenu = false;
  
  tft.fillScreen(ST7735_BLACK);
  drawLayout();
  
  nextType = random(TYPES);
  generate();
  
  timer = millis();
}

void generate() {
  currentType = nextType;
  nextType = random(TYPES);
  
  if (currentType != 5)
    pieceX = random(9);
  else
    pieceX = random(7);
    
  pieceY = 0;
  rotation = 0;
  copyPiece(piece, currentType, rotation);
  
  oldPieceX = pieceX;
  oldPieceY = pieceY;
  for (short i = 0; i < 4; i++) {
    oldPiece[0][i] = piece[0][i];
    oldPiece[1][i] = piece[1][i];
  }
  
  drawNextPiece();
}

void drawNextPiece() {
  tft.fillRect(110, 9, 50, 20, ST7735_BLACK);
  
  short nPiece[2][4];
  copyPiece(nPiece, nextType, 0);
  
  for (short i = 0; i < 4; i++) {
    tft.fillRect(110 + 4 * nPiece[0][i], 
                 9 + 4 * nPiece[1][i], 
                 3, 3, PIECE_COLORS[nextType]);
  }
}

void copyPiece(short piece[2][4], short type, short rotation) {
  switch (type) {
    case 0:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_L_l[rotation][0][i];
        piece[1][i] = pieces_L_l[rotation][1][i];
      }
      break;
    case 1:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_l[rotation][0][i];
        piece[1][i] = pieces_S_l[rotation][1][i];
      }
      break;
    case 2:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_r[rotation][0][i];
        piece[1][i] = pieces_S_r[rotation][1][i];
      }
      break;
    case 3:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_Sq[0][0][i];
        piece[1][i] = pieces_Sq[0][1][i];
      }
      break;
    case 4:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_T[rotation][0][i];
        piece[1][i] = pieces_T[rotation][1][i];
      }
      break;
    case 5:
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_l[rotation][0][i];
        piece[1][i] = pieces_l[rotation][1][i];
      }
      break;
  }
}

short getMaxRotation(short type) {
  // Return maximum rotation states for each piece type
  if (type == 1 || type == 2 || type == 5)      // S, S_r, I-pieces: 2 states
    return 2;
  else if (type == 0 || type == 4)              // L, T-pieces: 4 states
    return 4;
  else if (type == 3)                           // Square: 1 state (no rotation)
    return 1;
  else
    return 0;
}

boolean canRotate(short rotation) {
  short testPiece[2][4];
  copyPiece(testPiece, currentType, rotation);
  return !nextHorizontalCollision(testPiece, 0);
}

void updateScore() {
  if (score != oldScore) {
    tft.fillRect(45, 4, 20, 8, ST7735_BLACK);
    
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.setCursor(45, 4);
    tft.print(score);
    
    oldScore = score;
  }
}

void drawLayout() {
  tft.drawRect(MARGIN_LEFT - 1, MARGIN_TOP - 1, 63, 111, ST7735_WHITE);
  
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(5, 4);
  tft.print("SCORE:");
  
  tft.setCursor(75, 4);
  tft.print("NEXT:");
  
  updateScore();
}

void updatePiece() {
  erasePiece(oldPiece, oldPieceX, oldPieceY);
  drawPiece(currentType, pieceX, pieceY);
  
  oldPieceX = pieceX;
  oldPieceY = pieceY;
  for (short i = 0; i < 4; i++) {
    oldPiece[0][i] = piece[0][i];
    oldPiece[1][i] = piece[1][i];
  }
}

void tetrisSetup() {
  tft.fillScreen(ST7735_BLACK);
  
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.print("TETRIS");
  delay(1500);
  
  resetGame();
}

void tetrisLoop() {
  // Handle game over screen
  if (gameOver) {
    if (!digitalRead(joyButton) || analogRead(joyY) < 512 - JOY_THRESHOLD || 
        analogRead(joyY) > 512 + JOY_THRESHOLD || analogRead(joyX) < 512 - JOY_THRESHOLD || 
        analogRead(joyX) > 512 + JOY_THRESHOLD || !digitalRead(button)) {
      delay(500);
      returnToMenu = true;
    }
    return;
  }
  
  if (millis() - timer > interval) {
    checkLines();
    updateScore();
    
    if (nextCollision()) {
      // Place piece in grid
      for (short i = 0; i < 4; i++) {
        grid[pieceX + piece[0][i]][pieceY + piece[1][i]] = 1;
        gridColors[pieceX + piece[0][i]][pieceY + piece[1][i]] = PIECE_COLORS[currentType];
      }
      
      if (checkGameOver()) {
        showGameOver();
        return;
      }
      
      generate();  // Spawn next piece
      drawPiece(currentType, pieceX, pieceY);
    } else {
      pieceY++;  // Move piece down
      updatePiece();
    }
    timer = millis();
  }
  
  int x = analogRead(joyX);
  int y = analogRead(joyY);
  
  // Handle horizontal movement
  if (millis() - lastMoveTime > MOVE_DELAY) {
    if (x > 512 + JOY_THRESHOLD) {  // Move left
      if (!nextHorizontalCollision(piece, -1)) {
        pieceX--;
        updatePiece();
        lastMoveTime = millis();
      }
    } else if (x < 512 - JOY_THRESHOLD) {  // Move right
      if (!nextHorizontalCollision(piece, 1)) {
        pieceX++;
        updatePiece();
        lastMoveTime = millis();
      }
    }
    
    // Soft drop: accelerate on down joystick
    if (y < 512 - JOY_THRESHOLD) {
      interval = 50;
    } else {
      interval = 400;
    }
  }
  
  if (!digitalRead(joyButton) || y > 512 + JOY_THRESHOLD || !digitalRead(button)) {
    if (b3) {
      short newRotation = rotation;
      
      if (rotation == getMaxRotation(currentType) - 1 && canRotate(0)) {
        newRotation = 0;
      } else if (canRotate(rotation + 1)) {
        newRotation = rotation + 1;
      }
      
      if (newRotation != rotation) {
        rotation = newRotation;
        copyPiece(piece, currentType, rotation);
        updatePiece();
      }
      
      b3 = false;
      delayer = millis();
    }
  } else if (millis() - delayer > 200) {
    b3 = true;
  }
}

bool tetrisCheckReturnToMenu() {
  return returnToMenu;
}