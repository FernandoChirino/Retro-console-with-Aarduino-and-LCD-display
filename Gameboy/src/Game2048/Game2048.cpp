#include "Game2048.h"

#define Pushbutton 8

int JOY_THRESHOLD = 300;
bool over = false;

Game2048::Game2048(int cs, int dc, int rst) : tft(cs, dc, rst) {
  moveInProgress = false;
}

void Game2048::begin() {
  // Initialize ST7735 display (128x128)
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  
  // Setup joystick pins
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(Pushbutton, INPUT_PULLUP);
  
  // Initialize board with zeros
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      board[i][j] = 0;
    }
  }

  // First two tiles on the board
  placeNewTile();
  placeNewTile();
  
  showBoard();
}

bool Game2048::update() {

  if (digitalRead(Pushbutton) == LOW) {
    return false;
  }

  handleJoystick();
  return true;
}

void Game2048::handleJoystick() {
  // If the game is not over yet
  if(gameStatus() != 2) {
    over = false;
    // Read joystick values
    int joyX = analogRead(JOY_X);
    int joyY = analogRead(JOY_Y);
    
    // Only process movement if not already moving (debounce)
    if(!moveInProgress) {
      bool moved = false;
      
      // Check horizontal movement (X-axis)
      if(joyX < (JOY_CENTER - JOY_THRESHOLD)) {
        // Left
        if(right()) { placeNewTile(); }
        moved = true;
      }
      else if(joyX > (JOY_CENTER + JOY_THRESHOLD)) {
        // Right
        if(left()) { placeNewTile(); }
        moved = true;
      }
      // Check vertical movement (Y-axis)
      else if(joyY < (JOY_CENTER - JOY_THRESHOLD)) {
        // Up
        if(down()) { placeNewTile(); }
        moved = true;
      }
      else if(joyY > (JOY_CENTER + JOY_THRESHOLD)) {
        // Down
        if(up()) { placeNewTile(); }
        moved = true;
      }
      
      if(moved) {
        showBoard();
        moveInProgress = true;
        delay(300);  // Delay to prevent multiple moves
      }
    }
    else {
      // Reset move flag when joystick returns to center
      if(abs(joyX - JOY_CENTER) < JOY_DEADZONE && 
         abs(joyY - JOY_CENTER) < JOY_DEADZONE) {
        moveInProgress = false;
      }
    }
    
    // Optional: Use joystick button to restart game
    if(digitalRead(JOY_BTN) == LOW) {
      resetGame();
      delay(300);
    }
  }
  else {
    // Game over - display message
    if (!over) {
      displayGameOver();
    }
    over = true;
    
    // Press joystick button to restart
    if(digitalRead(JOY_BTN) == LOW) {
      resetGame();
      over = false;
      delay(300);
    }
  }
}

// ---- RANDOM HELPER FUNCTIONS ----
int Game2048::newTileValue() {
  int chance = random(10);
  if(chance < 8) { return 2; }
  else { return 4; }
}

int Game2048::getRandomPoint() {
  return random(4);
}

// ---- COLOR FUNCTIONS ----
uint16_t Game2048::getTileColor(int tileValue) {
  switch(tileValue) {
    case    0: return TILE_EMPTY;
    case    2: return TILE_2;
    case    4: return TILE_4;
    case    8: return TILE_8;
    case   16: return TILE_16;
    case   32: return TILE_32;
    case   64: return TILE_64;
    case  128: return TILE_128;
    case  256: return TILE_256;
    case  512: return TILE_512;
    case 1024: return TILE_1024;
    case 2048: return TILE_2048;
    default: return MAGENTA;
  }
}

uint16_t Game2048::getTextColor(int tileValue) {
  if(tileValue <= 4) {
    return 0x5ACB;  // Dark brown/grey
  }
  return WHITE;
}

bool Game2048::isValidPosition(int xpos, int ypos) {
  return xpos < 4 && ypos < 4 && xpos >= 0 && ypos >= 0;
}

// ---- DISPLAY FUNCTIONS ----
void Game2048::drawTile(int x, int y, int value) {
  // Calculate pixel position
  int px = BOARD_OFFSET + x * (TILE_SIZE + TILE_MARGIN);
  int py = BOARD_OFFSET + y * (TILE_SIZE + TILE_MARGIN);
  
  // Get tile color based on value
  uint16_t tileColor = getTileColor(value);
  
  // Draw tile background
  tft.fillRoundRect(px, py, TILE_SIZE, TILE_SIZE, 3, tileColor);
  
  // Draw tile value if not empty
  if(value > 0) {
    // Set text color based on tile value
    uint16_t textColor = getTextColor(value);
    tft.setTextColor(textColor);
    
    // Determine text size based on number of digits
    int textSize;
    if(value < 10) {
      textSize = 2;  // Single digit
    } else if(value < 100) {
      textSize = 2;  // Two digits
    } else if(value < 1000) {
      textSize = 1;  // Three digits
    } else {
      textSize = 1;  // Four+ digits
    }
    
    tft.setTextSize(textSize);
    
    // Center the text
    String valueStr = String(value);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
    
    int textX = px + (TILE_SIZE - w) / 2;
    int textY = py + (TILE_SIZE - h) / 2 + 1;
    
    tft.setCursor(textX, textY);
    tft.print(value);
  }
}

void Game2048::showBoard() {
  tft.fillScreen(ST77XX_BLACK);
  
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      drawTile(i, j, board[i][j]);
    }
  }
}

void Game2048::displayGameOver() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 55);
  tft.println("GAME OVER");
  tft.setTextSize(1);
  tft.setCursor(15, 75);
  tft.println("Press button");
  tft.setCursor(25, 85);
  tft.println("to restart");
}

// ---- GAME LOGIC FUNCTIONS ----
int Game2048::gameStatus() {
  bool moveable = false;
  int x, y;
  for (y = 0; y <= 3; y++) {
    for (x = 0; x <= 3; x++) {
      if (board[x][y] == 0) {
        return 0; // board not full
      }
      if (x <= 2) {
        if (board[x][y] == board[x+1][y])
          moveable = true;
      }
      if (y <= 2) {
        if (board[x][y] == board[x][y+1])
          moveable = true;
      }
    }
  }
  if (moveable) return 1;
  else return 2;
}

void Game2048::placeNewTile() {
  if (gameStatus() == 0) {
    int x, y, i;
    for (i = 1; i <= 30; i++) {
      x = getRandomPoint();
      y = getRandomPoint();
      if (!board[x][y]) {
         board[x][y] = newTileValue();
         return;
      }
    }
    for (y = 0; y <= 3; y++) {
      for (x = 0; x <= 3; x++) {
        if (!board[x][y]) {
           board[x][y] = newTileValue();
           return;
        }
      }
    }
  }
}

void Game2048::resetGame() {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      board[i][j] = 0;
    }
  }
  placeNewTile();
  placeNewTile();
  showBoard();
}

int Game2048::moveOnce(int x, int y, bool h, bool v, bool hv, bool vv, bool noMerge) {
  int tx = x, ty = y;
  
  if (h) {
    if (hv) tx += 1;
    else tx -= 1;
  }
  
  if (v) {
    if (vv) ty += 1;
    else ty -= 1;
  }
  
  if(!isValidPosition(x,y) || !isValidPosition(tx,ty)) {
    return 0;
  }
  
  else if (board[x][y] == 0) { return 0; }
  else if (board[tx][ty] == 0) {
    board[tx][ty] = board[x][y];
    board[x][y] = 0;
    return 1;
  } 
  else if (!noMerge && board[tx][ty] == board[x][y]) {
    board[tx][ty] = 2 * board[tx][ty];
    board[x][y] = 0;
    return 2;
  }
  return 0;
}

#define moveVerticallyOnce(x, y, d, noMerge) moveOnce(x, y, false, true, false, d, noMerge)
#define moveHorizontallyOnce(x, y, d, noMerge) moveOnce(x, y, true, false, d, false, noMerge)

bool Game2048::up() {
  int x, y, ly, lyMargin, result = 0;
  bool moved = false;
  bool noMerge = false;
  for (x = 0; x <= 3; x++) {
    lyMargin = 0;
    for (y = 1; y <= 3; y++) {
      noMerge = false;
      for (ly = y; ly > lyMargin; ly--) {
        result = moveVerticallyOnce(x, ly, false, noMerge);
        if (result)
          moved = true;
        if (result == 2) {
          noMerge = true;
          lyMargin = ly;
        }
        if (result == 0) break;
      }
    }
  }
  return moved;
}

bool Game2048::left() {
  int x, y, lx, lxMargin, result = 0;
  bool moved = false;
  bool noMerge = false;
  for (y = 0; y <= 3; y++) {
    lxMargin = 0;
    for (x = 1; x <= 3; x++) {
      noMerge = false;
      for (lx = x; lx > lxMargin; lx--) {
        result = moveHorizontallyOnce(lx, y, false, noMerge);
        if (result)
          moved = true;
        if (result == 2) {
          noMerge = true;
          lxMargin = lx;
        }
        if (result == 0) break;
      }
    }
  }
  return moved;
}

bool Game2048::down() {
  int x, y, ly, lyMargin, result = 0;
  bool moved = false;
  bool noMerge = false;
  for (x = 0; x <= 3; x++) {
    lyMargin = 3;
    for (y = 2; y >= 0; y--) {
      noMerge = false;
      for (ly = y; ly < lyMargin; ly++) {
        result = moveVerticallyOnce(x, ly, true, noMerge);
        if (result)
          moved = true;
        if (result == 2) {
          noMerge = true;
          lyMargin = ly;
        }
        if (result == 0) break;
      }
      if (y == 0) break;
    }
  }
  return moved;
}

bool Game2048::right() {
  int x, y, lx, lxMargin, result = 0;
  bool moved = false;
  bool noMerge = false;
  for (y = 0; y <= 3; y++) {
    lxMargin = 3;
    for (x = 2; x >= 0; x--) {
      noMerge = false;
      for (lx = x; lx < lxMargin; lx++) {
        result = moveHorizontallyOnce(lx, y, true, noMerge);
        if (result)
          moved = true;
        if (result == 2) {
          noMerge = true;
          lxMargin = lx;
        }
        if (result == 0) break;
      }
      if (x == 0) break;
    }
  }
  return moved;
}
