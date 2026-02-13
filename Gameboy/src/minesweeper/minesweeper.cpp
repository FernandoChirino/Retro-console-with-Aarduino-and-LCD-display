#include "minesweeper.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ST7735 pins for Arduino Mega
#define TFT_CS     6  // Chip select
#define TFT_RST    7   // Reset
#define TFT_DC     5   // Data/Command

// Joystick pins
#define JOY_X A5
#define JOY_Y A4
#define JOY_SW 8
#define PushButton 4

#define blockSize 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define GRID_SIZE SCREEN_WIDTH / blockSize
#define NUM_BOMBS 10

// Custom Colors
#define Background_gray 0xe73c
#define Outline_gray 0xbdf7
#define Revealed_gray 0xdedb

// Button
const unsigned long HOLD_THRESHOLD = 200;
unsigned long pressStartTime = 0;
bool wasPressed = false;
bool holdTriggered = false;

// Movements
int current_position[2];
enum Direction { UP, DOWN, LEFT, RIGHT };

// Game
bool win = false;
bool lose = false;

// Game grid
int game_grid[GRID_SIZE][GRID_SIZE];
int reveal_grid[GRID_SIZE][GRID_SIZE] = {0};

void GameOver()
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED);
  tft.setCursor(10, 60);
  tft.println("GAME OVER");
  tft.setTextSize(1);
}

void winGame()
{
  tft.fillScreen(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 60);
  tft.println("VICTORY");
  tft.setTextSize(1);
}

bool checkWin() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      // If it's not a mine AND not revealed, game isn't won yet
      if (game_grid[i][j] != -1 && reveal_grid[i][j] != 1) {
        return false;
      }
    }
  }
  return true;  // All non-mine tiles revealed!
}

void drawFlag(int gridX, int gridY) {
  int x = gridX * blockSize;
  int y = gridY * blockSize;
  
  // Flagpole
  tft.fillRect(x + 8, y + 2, 1, 12, ST77XX_BLACK);

  tft.fillRect(x + 4, y + 14, 9, 2, ST77XX_BLACK);
  
  // Flag (filled triangle pointing left)
  tft.fillTriangle(x + 8, y + 3, x + 8, y + 10, x + 3, y + 6, ST77XX_RED);
  
}

void drawCenteredNumber(int gridX, int gridY, int number, int textSize) {
  int charWidth = 6 * textSize;
  int charHeight = 8 * textSize;
  int tileSize = 16;
  
  int offsetX = (tileSize - charWidth) / 2;
  int offsetY = (tileSize - charHeight) / 2;

  if (number == 0){
       tft.fillRect(gridX * tileSize, gridY * tileSize, blockSize, blockSize, Revealed_gray);
       tft.drawRect(gridX * tileSize, gridY * tileSize, blockSize, blockSize, Outline_gray);
       return;
  }

  else if (number == 1) tft.setTextColor(ST77XX_BLUE);
  else if (number == 2) tft.setTextColor(ST77XX_RED);
  else if (number == 3) tft.setTextColor(ST77XX_GREEN);
  else if (number == 4) tft.setTextColor(ST77XX_MAGENTA);
  else tft.setTextColor(ST77XX_BLACK);

  tft.setTextSize(textSize);
  tft.setCursor(gridX * tileSize + offsetX, gridY * tileSize + offsetY);
  tft.fillRect(gridX * tileSize, gridY * tileSize, blockSize, blockSize, Background_gray);
  tft.drawRect(gridX * tileSize, gridY * tileSize, blockSize, blockSize, Outline_gray);
  tft.print(number);
}

// Randomly generate bombs and add the numeber for all adjacent tiles.
void generateBombs() {
  // Initialize grid to all zeros
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      game_grid[x][y] = 0;
    }
  }
  
  // Place bombs randomly
  int bombsPlaced = 0;
  while (bombsPlaced < NUM_BOMBS) {
    int x = random(0, GRID_SIZE);
    int y = random(0, GRID_SIZE);
    
    // Only place bomb if cell is empty
    if (game_grid[x][y] != -1) {
      game_grid[x][y] = -1;  // -1 represents a bomb
      bombsPlaced++;
    }
  }
  
  // Calculate numbers for non-bomb cells
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      if (game_grid[x][y] != -1) {  // If not a bomb
        int count = 0;
        
        // Check all 8 adjacent cells
        for (int dx = -1; dx <= 1; dx++) {
          for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;  // Skip center cell
            
            int nx = x + dx;
            int ny = y + dy;
            
            // Check if adjacent cell is within bounds and is a bomb
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
              if (game_grid[nx][ny] == -1) {
                count++;
              }
            }
          }
        }
        
        game_grid[x][y] = count;  // Store number of adjacent bombs
      }
    }
  }
}

void floodFill(int x, int y) 
{
  // Mark as revealed
  reveal_grid[x][y] = 1; 

  drawCenteredNumber(x, y, game_grid[x][y], 2);

  // Check all 8 adjacent cells
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;

      int nx = x + dx;
      int ny = y + dy;

      //bounds checking
      if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
      
      // check if already revealed
      if (reveal_grid[nx][ny] == 1) continue;

      if (game_grid[nx][ny] == 0) {
        floodFill(nx, ny);  // Recurse on 0 tiles
      }
      else {
        // Reveal numbered tiles at the edge
        reveal_grid[nx][ny] = 1;
        drawCenteredNumber(nx, ny, game_grid[nx][ny], 2);
      }
    }
  }
}

void setupMinesweeper() {
  // Setup arduino pins
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(PushButton, INPUT_PULLUP);
  randomSeed(analogRead(A2));

  // Reset selected tile to top left corner
  current_position[0] = 0;
  current_position[1] = 0;

  generateBombs();

  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      reveal_grid[x][y] = 0;
    }
  }

  // Draw nackground and tiles
  tft.fillScreen(ST77XX_WHITE);

  for (int x = 0; x <= SCREEN_WIDTH; x += blockSize) {
    for (int y = 0; y <= SCREEN_HEIGHT; y += blockSize) {

      tft.fillRect(x, y, blockSize, blockSize, Background_gray);
      tft.drawRect(x, y, blockSize, blockSize, Outline_gray);
    }
  }
}

bool loopMinesweeper() {
  // Tile positions of previous selected square
  int x = current_position[0] * blockSize;
  int y = current_position[1] * blockSize;

  if (digitalRead(JOY_SW) == LOW) {
      return false;  // Signal to exit
  }

  if (checkWin() == true && win == false)
  {
    win = true;
    winGame();
  }

  if (lose == true || win == true)
{
  if (digitalRead(PushButton) == LOW){
    setupMinesweeper();
    lose = false;
    win = false;
    
    while (digitalRead(PushButton) == LOW) {
      delay(10);
    }
    wasPressed = false;
  }
  return true; 
}

  if (win == false && lose == false)
  {
  // Erase previous selected square
    tft.drawRect(x, y, blockSize, blockSize, Outline_gray);

        // Read joystick
    int xVal = analogRead(JOY_X);
    int yVal = analogRead(JOY_Y);
    if (xVal < 400 && current_position[0] < 7) current_position[0]++;
    else if (xVal > 600 && current_position[0] > 0) current_position[0]--;
    else if (yVal < 400 && current_position[1] < 7) current_position[1]++;
    else if (yVal > 600 && current_position[1] > 0) current_position[1]--;

    // Update current tile position
    x = current_position[0] * blockSize;
    y = current_position[1] * blockSize;

    // Check button press

    bool isPressed = (digitalRead(PushButton) == LOW);
    
    if (isPressed && !wasPressed) {
      pressStartTime = millis();
      wasPressed = true;
      holdTriggered = false; // Reset flag
    }
    
    // Check if still holding (Hold press)
    if (isPressed && wasPressed && !holdTriggered) {
      if (millis() - pressStartTime >= HOLD_THRESHOLD) {

        drawFlag(current_position[0], current_position[1]);
        
        holdTriggered = true;
      }
    }
    
    // Quick press - reveal tile
    if (!isPressed && wasPressed) {
      if (!holdTriggered) {

        int current_tile_value = game_grid[current_position[0]][current_position[1]]; // Read current position in game grid

        // BOMB
        if (current_tile_value == -1) {
          reveal_grid[current_position[0]][current_position[1]] = 1;
          tft.fillRect(x, y, blockSize, blockSize, ST77XX_RED);
          tft.fillRect(x + 4, y + 4, blockSize - 8, blockSize - 8, ST77XX_BLACK);
          lose = true;
          delay(100);
          GameOver();
          return true;
        }

        else {
          // If 0, apply flood fill
          if (current_tile_value == 0) floodFill(current_position[0], current_position[1]);

          // Else, reveal tile
          else {
            reveal_grid[current_position[0]][current_position[1]] = 1;   
            drawCenteredNumber(current_position[0], current_position[1], current_tile_value, 2);
          }

        }
      }
      wasPressed = false;
    }
    

    // Draw current selected square
    tft.drawRect(current_position[0] * blockSize, current_position[1] * blockSize, blockSize, blockSize, ST77XX_RED);
  }
  
  delay(150);
  return true;
}

