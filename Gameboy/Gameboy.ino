#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "tetris.h"

// ST7735 Display pins
#define TFT_CS     53  // Chip Select
#define TFT_RST    31  // Reset
#define TFT_DC     33  // Data/Command   

// Joystick pins 
const int joyX = A0;     
const int joyY = A1;     
const int joyButton = 2; 
const int button = 27;

// Initialize display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Game state machine
enum GameState {
  MENU,
  TETRIS,
  SNAKE,
  DOGEBLOCK  // Add more games here
};

GameState currentState = MENU; // Current active screen/game
int menuSelection = 0;          // Currently highlighted menu item
const int NUM_GAMES = 3;        // Number of playable games
unsigned long lastMenuMove = 0; // Debounce timer for menu navigation
const int MENU_DELAY = 200;     // Minimum time between menu moves (ms)
const int JOY_THRESHOLD = 200;  // Joystick deadzone threshold

void drawMenu() {
  tft.fillScreen(ST7735_BLACK);
  
  // Title
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(15, 20);
  tft.print("RETRO");
  tft.setCursor(10, 38);
  tft.print("CONSOLE");
  
  // Instructions
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(10, 120);
  tft.print("Move to select");
  
  // Draw all menu items
  drawMenuItem(0);
  drawMenuItem(1);
  drawMenuItem(2);
}

void drawMenuItem(int itemIndex) {
  int yPos = 70 + (itemIndex * 15);  // 15 pixels spacing between items
  
  // Clear the item area
  tft.fillRect(10, yPos, 108, 12, ST7735_BLACK);
  
  // Draw selection highlight
  if (menuSelection == itemIndex) {
    tft.fillRect(10, yPos, 108, 12, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK);
  } else {
    tft.setTextColor(ST7735_WHITE);
  }
  
  tft.setTextSize(1);
  
  // Draw the appropriate game name
  switch (itemIndex) {
    case 0:
      tft.setCursor(30, yPos + 2);
      tft.print("1. TETRIS");
      break;
    case 1:
      tft.setCursor(30, yPos + 2);
      tft.print("2. SNAKE");
      break;
    case 2:
      tft.setCursor(30, yPos + 2);
      tft.print("3. DODGE BLOCK");
      break;
  }
}

void handleMenu() {
  int y = analogRead(joyY);
  
  // Navigate menu
  if (millis() - lastMenuMove > MENU_DELAY) {
    if (y > 512 + JOY_THRESHOLD) {  // Up
      int oldSelection = menuSelection;
      menuSelection--;
      if (menuSelection < 0) menuSelection = NUM_GAMES - 1;
      
      // Only redraw the affected items
      drawMenuItem(oldSelection);
      drawMenuItem(menuSelection);
      
      lastMenuMove = millis();
    } else if (y < 512 - JOY_THRESHOLD) {  // Down
      int oldSelection = menuSelection;
      menuSelection++;
      if (menuSelection >= NUM_GAMES) menuSelection = 0;
      
      // Only redraw the affected items
      drawMenuItem(oldSelection);
      drawMenuItem(menuSelection);
      
      lastMenuMove = millis();
    }
  }
  
  // Select game
  if (!digitalRead(joyButton) || !digitalRead(button)) {
    delay(300);  // Debounce
    
    switch (menuSelection) {
      case 0:
        currentState = TETRIS;
        tetrisSetup();
        break;
      // Add more cases for other games
      /*
      case 1:
        currentState = GAME2;
        game2Setup();
        break;
      */
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(joyButton, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  
  // Initialize display
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(3);
  
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
  /*
  Serial.print("X: ");
  Serial.print(analogRead(joyX));
  Serial.print("\t");
  Serial.print("Y: ");
  Serial.println(analogRead(joyY));
  */

  switch (currentState) {
    case MENU:
      handleMenu();
      break;
      
    case TETRIS:
      tetrisLoop();
      // Check if player wants to return to menu (implement in tetris)
      if (tetrisCheckReturnToMenu()) {
        currentState = MENU;
        drawMenu();
      }
      break;
      
    // Add more game cases here
    /*
    case SNAKE:
      game2Loop();
      break;
    */
  }
}