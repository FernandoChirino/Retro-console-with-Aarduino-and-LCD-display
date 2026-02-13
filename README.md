# Retro Console Gaming System

A multi-game handheld console built with Arduino and ST7735 TFT display, featuring 6 classic arcade games.

## Hardware Requirements

- Arduino MEGA board (Uno/Nano compatible)
- ST7735 TFT Display (128x128, 1.44")
- Analog joystick module
- 2 push buttons
- Jumper wires and breadboard

## Pin Configuration

### Display (ST7735)
- CS (Chip Select): Pin 6
- RST (Reset): Pin 7
- DC (Data/Command): Pin 5
- SDA/MOSI: Pin 11 (SPI)
- SCL/SCK: Pin 13 (SPI)

### Controls
- Joystick X-axis: A5
- Joystick Y-axis: A4
- Joystick Button: Pin 8
- Game Button: Pin 4

## Games Included

1. **Tetris** - Classic block-stacking puzzle
2. **Pac-Man** - Navigate mazes and collect dots
3. **Dodge Mario** - Avoid falling obstacles
4. **Snake** - Grow your snake without crashing
5. **Breakout** - Break bricks with a paddle
6. **2048** - Number tile puzzle game
7. **Minesweeper** - Classic single player logic game

## Features

- Paginated menu system (4 games per page)
- Joystick navigation (Up/Down: select game, Left/Right: change pages)
- Return to menu from any game
- Color-coded interface with splash screen
- Modular game architecture

## Installation

1. Install required libraries:
   - Adafruit GFX Library
   - Adafruit ST7735 Library
   
2. Clone or download this repository

3. Open `main.ino` in Arduino IDE

4. Upload to your Arduino board

## Controls

- **Joystick Up/Down**: Navigate menu selections
- **Joystick Left/Right**: Switch between menu pages
- **Game Button (Pin 4)**: Select game / In-game actions
- **Joystick Button (Pin 8)**: Additional game controls

## File Structure
```
retro-console/
├── main.ino
└── src/
    ├── tetris/
    ├── pacman/
    ├── dodge/
    ├── snake/
    └── breakout/
```

## Credits

### Game Adaptations

The games in this console are adapted from various open-source Arduino projects:

- Tetris: https://circuitdigest.com/microcontroller-projects/creating-tetris-game-with-arduino-and-oled-display
- Pac-Man: [github.com/dlallan/pac-man-arduino](https://github.com/dlallan/pac-man-arduino/tree/master)
- Dodge Mario: https://techcraftandhacks.in/esp32-joystick-game-oled-buzzer/
- Snake: [github.com/TechsPassion/Arduino/Snake_Game.ino](https://github.com/TechsPassion/Arduino/blob/main/Snake_Game.ino)
- Breakout: https://www.hackster.io/javagoza/arduino-touch-breakout-game-eda113 
- 2048: [github.com/peacheym/NeoPixel2048](https://github.com/peacheym/NeoPixel2048)

All games have been modified and integrated into a unified console system with custom menu navigation, standardized controls, and optimized display rendering.

Special thanks to the original authors for their amazing work.
