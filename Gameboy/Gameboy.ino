#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "src/tetris/tetris.h"
#include "src/dodge/dodge.h"
#include "src/pacman/global.h"
#include "src/snake/SnakeGame.h"
#include "src/breakout/breakout.h"

// External functions from main.cpp
extern void pacmanGameSetup();
extern bool pacmanGameLoop();

// ST7735 Display pins
#define TFT_CS     6  // Chip Select
#define TFT_RST    7  // Reset
#define TFT_DC     5  // Data/Command   

// Joystick pins 
const int joyX = A5;     
const int joyY = A4;     
const int joyButton = 8; 
const int button = 4;

// Initialize display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Game state machine
enum GameState {
  MENU,
  TETRIS,
  PACMAN,
  DODGEBLOCK,
  SNAKE,
  BREAKOUT,
  SPACE_INVADERS
};

GameState currentState = MENU;
int menuSelection = 0;
const int NUM_GAMES = 6;
const int GAMES_PER_PAGE = 4;  // How many games to show per page
int currentPage = 0;            // Current page index
const int NUM_PAGES = (NUM_GAMES + GAMES_PER_PAGE - 1) / GAMES_PER_PAGE;  // Calculate total pages

unsigned long lastMenuMove = 0;
const int MENU_DELAY = 200;
const int JOY_THRESHOLD = 200;

// Game names array 
const char* gameNames[] = {
  "1. TETRIS",
  "2. PACMAN",
  "3. DODGE MARIO",
  "4. SNAKE",
  "5. BREAKOUT",
  "6. SPACE INVADER",
}; 

void drawMenu() {
  tft.fillScreen(ST7735_BLACK);

  // Title
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(15, 5);
  tft.print("RETRO");
  tft.setCursor(10, 22);
  tft.print("CONSOLE");

  // Page indicator
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.setCursor(45, 40);
  tft.print("Page ");
  tft.print(currentPage + 1);
  tft.print("/");
  tft.print(NUM_PAGES);
  
  // Draw navigation arrows
  if (currentPage > 0) {
    tft.setTextColor(ST7735_YELLOW);
    tft.setCursor(10, 40);
    tft.print("<");
  }
  if (currentPage < NUM_PAGES - 1) {
    tft.setTextColor(ST7735_YELLOW);
    tft.setCursor(110, 40);
    tft.print(">");
  }
  
  // Instructions
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(5, 115);
  tft.print("Up/Down   Left/Right");
  
  // Draw menu items for current page
  int startGame = currentPage * GAMES_PER_PAGE;
  int endGame = min(startGame + GAMES_PER_PAGE, NUM_GAMES);
  
  for (int i = startGame; i < endGame; i++) {
    drawMenuItem(i);
  }
}


void drawMenuItem(int gameIndex) {
  // Calculate position relative to current page
  int positionOnPage = gameIndex - (currentPage * GAMES_PER_PAGE);
  int yPos = 55 + (positionOnPage * 15);  // 15 pixels spacing between items
  
  // Clear the item area
  tft.fillRect(10, yPos, 108, 12, ST7735_BLACK);
  
  // Draw selection highlight
  if (menuSelection == gameIndex) {
    tft.fillRect(10, yPos, 108, 12, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK);
  } else {
    tft.setTextColor(ST7735_WHITE);
  }
  
  tft.setTextSize(1);
  tft.setCursor(15, yPos + 2);
  tft.print(gameNames[gameIndex]);
}

void handleMenu() {
  int x = analogRead(joyX);
  int y = analogRead(joyY);
  
  // Navigate menu
  if (millis() - lastMenuMove > MENU_DELAY) {
    // Up/Down navigation within page
    if (y > 512 + JOY_THRESHOLD) {  // Up
      int oldSelection = menuSelection;
      menuSelection--;
      
      // Handle page wrapping
      if (menuSelection < 0) {
        menuSelection = NUM_GAMES - 1;
        currentPage = NUM_PAGES - 1;
        drawMenu();  // Redraw entire menu for page change
      } else if (menuSelection < currentPage * GAMES_PER_PAGE) {
        // Moved to previous page
        currentPage--;
        drawMenu();  // Redraw entire menu for page change
      } else {
        // Same page, just update items
        drawMenuItem(oldSelection);
        drawMenuItem(menuSelection);
      }
      
      lastMenuMove = millis();
      
    } else if (y < 512 - JOY_THRESHOLD) {  // Down
      int oldSelection = menuSelection;
      menuSelection++;
      
      // Handle page wrapping
      if (menuSelection >= NUM_GAMES) {
        menuSelection = 0;
        currentPage = 0;
        drawMenu();  // Redraw entire menu for page change
      } else if (menuSelection >= (currentPage + 1) * GAMES_PER_PAGE) {
        // Moved to next page
        currentPage++;
        drawMenu();  // Redraw entire menu for page change
      } else {
        // Same page, just update items
        drawMenuItem(oldSelection);
        drawMenuItem(menuSelection);
      }
      
      lastMenuMove = millis();
    }
    
    // Left/Right page navigation
    if (x > 512 + JOY_THRESHOLD) {  // Left - previous page
      if (currentPage > 0) {
        currentPage--;
        menuSelection = currentPage * GAMES_PER_PAGE;
        drawMenu();
        lastMenuMove = millis();
      }
      
    } else if (x < 512 - JOY_THRESHOLD) {  // Right - next page
      if (currentPage < NUM_PAGES - 1) {
        currentPage++;
        menuSelection = currentPage * GAMES_PER_PAGE;
        drawMenu();
        lastMenuMove = millis();
      }
    }
  }
  
  // Select game
  if (!digitalRead(button)) {
    delay(300);  // Debounce
    
    switch (menuSelection) {
      case 0:
        currentState = TETRIS;
        tetrisSetup();
        break;
      case 1:
        currentState = PACMAN;
        pacmanGameSetup();
        break;
      case 2:
        currentState = DODGEBLOCK;
        dodgeSetup();
        break;
      case 3:
        currentState = SNAKE;
        setupSnakeGame();
        break;
      case 4:
        currentState = BREAKOUT;
        breakoutSetup();
        break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(joyButton, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  
  // Initialize display
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(4);
  
  // Show splash screen
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(24, 50);
  tft.print("RETRO");
  tft.setCursor(24, 68);
  tft.print("CONSOLE");
  delay(2000);
  
  // Show menu
  drawMenu();
  
  randomSeed(analogRead(A2));
}

void loop() {
  switch (currentState) {
    case MENU:
      handleMenu();
      break;
      
    case TETRIS:
      tetrisLoop();
      if (tetrisCheckReturnToMenu()) {
        currentState = MENU;
        drawMenu();
      }
      break;

    case DODGEBLOCK:
      dodgeLoop();
      if (dodgeCheckReturnToMenu()) {
        currentState = MENU;
        drawMenu();
      }
      break;

    case PACMAN:
      if (pacmanGameLoop() == false) {
        currentState = MENU;
        drawMenu();
      }
      break;

    case SNAKE:
      if (loopSnakeGame() == false) {
        currentState = MENU;
        drawMenu();
      }
      break;
    
    case BREAKOUT:
      breakoutLoop();
      if (breakoutCheckReturnToMenu()) {
        currentState = MENU;
        drawMenu();
      }
      break;
  }
}
