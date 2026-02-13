//Part of this code was written with the help of AI
#include "SnakeGame.h"
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
#define JOY_SW 4
#define PushButton 8

// Game settings
#define SNAKE_BLOCK_SIZE 8  // Larger blocks for better visibility
#define MAX_SNAKE_LENGTH 64
#define HEADER_HEIGHT 12

// Screen dimensions (ST7735 is typically 128x160)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

// Colors
#define COLOR_BACKGROUND ST77XX_BLACK
#define COLOR_SNAKE ST77XX_GREEN
#define COLOR_FOOD ST77XX_RED
#define COLOR_TEXT ST77XX_WHITE
#define COLOR_BORDER ST77XX_BLUE

// Static variables
static bool prevPause = false;

enum Direction { UP, DOWN, LEFT, RIGHT };
static Direction currentDirection = RIGHT;

static int snakeX[MAX_SNAKE_LENGTH];
static int snakeY[MAX_SNAKE_LENGTH];
static int snakeLength = 3;

static int foodX, foodY;
static int score = 0;
static int lastScore = -1;
static unsigned long lastTime = 0;
static bool isPaused = false;
static bool buttonState = HIGH;
static unsigned long startTime;

// Forward declarations of helper functions
static void spawnFood();
static void drawInitialScreen();
static void updateHeader();
static void displayPause();
static void gameOver();

static void spawnFood() {
  int gridWidth = SCREEN_WIDTH / SNAKE_BLOCK_SIZE;
  int gridHeight = (SCREEN_HEIGHT - HEADER_HEIGHT) / SNAKE_BLOCK_SIZE;
  
  // Ensure food aligns with the grid
  bool foodOnSnake;
  do {
    foodOnSnake = false;
    foodX = random(0, gridWidth) * SNAKE_BLOCK_SIZE;
    foodY = HEADER_HEIGHT + random(0, gridHeight) * SNAKE_BLOCK_SIZE;
    
    // Check if food spawned on snake
    for (int i = 0; i < snakeLength; i++) {
      if (foodX == snakeX[i] && foodY == snakeY[i]) {
        foodOnSnake = true;
        break;
      }
    }
  } while (foodOnSnake);
}

static void drawInitialScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Draw initial header
  updateHeader();
}

static void updateHeader() {
  // Clear header area
  tft.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_BACKGROUND);
  
  // Draw score
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT);
  tft.setCursor(2, 2);
  tft.print("Score: ");
  tft.print(score);
  
  // Draw time
  tft.setCursor(72, 2);
  unsigned long elapsedSeconds = (millis() - startTime) / 1000;
  tft.print("T: ");
  tft.print(elapsedSeconds);
}

void setupSnakeGame() {
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(PushButton, INPUT_PULLUP);
  randomSeed(analogRead(A2));
  
  
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = (7 - i) * SNAKE_BLOCK_SIZE;  // Aligns to grid at position 7, 6, 5
    snakeY[i] = HEADER_HEIGHT + SNAKE_BLOCK_SIZE * 2;
  }

  spawnFood();
  startTime = millis();
  
  drawInitialScreen();
  
  // Draw initial food
  tft.fillRect(foodX, foodY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_FOOD);
  
  // Draw initial snake
  for (int i = 0; i < snakeLength; i++) {
    tft.fillRect(snakeX[i], snakeY[i], SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE);
  }
}

bool loopSnakeGame() {
  if (digitalRead(PushButton) == LOW) {
        return false;  // Signal to exit
    }
    
  // Pause toggle with debounce
  bool currentButtonState = digitalRead(JOY_SW);
  if (buttonState == HIGH && currentButtonState == LOW) {
    isPaused = !isPaused;
    
    if (isPaused) {
      // Just entered pause mode - draw pause screen once
      displayPause();
    } else {
      // Unpausing - redraw the game
      drawInitialScreen();
      // Redraw food
      tft.fillRect(foodX, foodY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_FOOD);
      // Redraw entire snake
      for (int i = 0; i < snakeLength; i++) {
        tft.fillRect(snakeX[i], snakeY[i], SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE);
      }
    }
    
    delay(200); // debounce
  }
  buttonState = currentButtonState;

  if (isPaused) {
    return true;  // Just return, don't redraw pause screen
  }


  // Read joystick
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  if (xVal < 450 && currentDirection != LEFT) currentDirection = RIGHT;
  else if (xVal > 550 && currentDirection != RIGHT) currentDirection = LEFT;
  else if (yVal < 450 && currentDirection != UP) currentDirection = DOWN;
  else if (yVal > 550 && currentDirection != DOWN) currentDirection = UP;

  // Store old tail position before moving
  int oldTailX = snakeX[snakeLength - 1];
  int oldTailY = snakeY[snakeLength - 1];

  // Move snake
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  switch (currentDirection) {
    case UP:    snakeY[0] -= SNAKE_BLOCK_SIZE; break;
    case DOWN:  snakeY[0] += SNAKE_BLOCK_SIZE; break;
    case LEFT:  snakeX[0] -= SNAKE_BLOCK_SIZE; break;
    case RIGHT: snakeX[0] += SNAKE_BLOCK_SIZE; break;
  }

  // Wall collision
  if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH ||
      snakeY[0] < HEADER_HEIGHT || snakeY[0] >= SCREEN_HEIGHT) {
    gameOver();
    return true;
  }

  // Self collision
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver();
      return true;
    }
  }

  // Check for food collision
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    score++;
    // Erase old food first
    tft.fillRect(foodX, foodY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_BACKGROUND);
    
    // Grow snake by keeping the tail
    if (snakeLength < MAX_SNAKE_LENGTH) {
      snakeX[snakeLength] = oldTailX;
      snakeY[snakeLength] = oldTailY;
      snakeLength++;
    }
    
    spawnFood();
    // Draw new food
    tft.fillRect(foodX, foodY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_FOOD);
  } else {
    // Only erase tail if we didn't eat food
    tft.fillRect(oldTailX, oldTailY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_BACKGROUND);
  }
  
  // === Efficient Drawing ===
  
  // Draw new head
  tft.fillRect(snakeX[0], snakeY[0], SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE);
  
  // Update header only when score changes or every second
  unsigned long currentTime = millis() / 1000;
  if (score != lastScore || currentTime != lastTime) {
    updateHeader();
    lastScore = score;
    lastTime = currentTime;
  }

  delay(150);
  return true;
}

static void displayPause() {
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextSize(3);
  tft.setTextColor(COLOR_TEXT);
  tft.setCursor(15, 70);
  tft.println("PAUSED");
}

static void gameOver() {
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED);
  tft.setCursor(10, 60);
  tft.println("GAME OVER");
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT);
  tft.setCursor(30, 85);
  tft.print("Score: ");
  tft.println(score);
  delay(3000);

  // Reset game
  snakeLength = 3;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = (7 - i) * SNAKE_BLOCK_SIZE;  // Was: 30 - i * SNAKE_BLOCK_SIZE
    snakeY[i] = HEADER_HEIGHT + SNAKE_BLOCK_SIZE * 2;
  }
  score = 0;
  lastScore = -1;
  startTime = millis();
  currentDirection = RIGHT;
  spawnFood();
  
  // Redraw everything
  drawInitialScreen();
  tft.fillRect(foodX, foodY, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_FOOD);
  for (int i = 0; i < snakeLength; i++) {
    tft.fillRect(snakeX[i], snakeY[i], SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE);
  }
}
