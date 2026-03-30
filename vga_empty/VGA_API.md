# VGA API Guide

## Purpose

This project exposes a small VGA drawing API so gameplay code does not need to know anything about:

- VGA timing
- DMA setup
- HSYNC / VSYNC generation
- scanline scheduling

Game code should only include `vga.h` and call the drawing functions.

## Main Files

- `Core/Inc/vga.h`
  Public API for teammates
- `Core/Src/vga.c`
  VGA driver and drawing implementation
- `Core/Src/main.c`
  Example of how to initialize and use the API

## Coordinate System

The API uses a logical game resolution:

- `VGA_WIDTH = 100`
- `VGA_HEIGHT = 75`

Top-left is `(0, 0)`.

- `x` grows to the right
- `y` grows downward

This logical image is expanded by the driver to fit the current VGA output timing, so teammates can think in simple game coordinates instead of raw scanline timings.

## Colors

Current supported colors are:

- `VGA_COLOR_BLACK`
- `VGA_COLOR_RED`
- `VGA_COLOR_GREEN`
- `VGA_COLOR_YELLOW`

At the moment the exposed drawing API is based on the active red/green signal lines, so blue is not part of the public color set yet.

## Initialization Flow

Typical startup order:

```c
#include "vga.h"

VGA_Init();
VGA_Clear(VGA_COLOR_BLACK);
VGA_DrawRect(0, 0, VGA_WIDTH, VGA_HEIGHT, VGA_COLOR_GREEN);
VGA_Start();
```

Recommended meaning of each step:

1. `VGA_Init()`
   Prepare the internal framebuffer and driver state.
2. Draw your initial scene
   Example: background, border, sprites, UI.
3. `VGA_Start()`
   Start the actual VGA signal output.

## Public API

### `void VGA_Init(void);`

Initializes the VGA driver state and framebuffer.

Use this once during startup before any drawing.

### `void VGA_Start(void);`

Starts VGA output.

Use this once after initialization and after you have prepared the first frame.

### `void VGA_Clear(VGA_Color color);`

Fills the entire logical screen with one color.

Typical use:

- clear the frame before drawing a new screen
- change background color

### `void VGA_DrawPixel(int16_t x, int16_t y, VGA_Color color);`

Draws one logical pixel.

Useful for:

- particle effects
- cursor/debug markers
- custom sprite routines

If the coordinate is outside the screen, the function safely does nothing.

### `void VGA_DrawHLine(int16_t x, int16_t y, int16_t length, VGA_Color color);`

Draws one horizontal line.

Useful for:

- UI bars
- borders
- health meters
- platforms

The function clips safely at screen boundaries.

### `void VGA_DrawVLine(int16_t x, int16_t y, int16_t length, VGA_Color color);`

Draws one vertical line.

Useful for:

- walls
- borders
- separators in menus

The function clips safely at screen boundaries.

### `void VGA_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color);`

Draws a rectangle outline.

Useful for:

- menu panels
- player boundary boxes
- HUD windows
- map tiles with hollow borders

### `void VGA_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color);`

Draws a filled rectangle.

Useful for:

- player blocks
- enemies
- bullets
- tile map cells
- buttons and UI backgrounds

The function clips safely at screen boundaries.

### `void VGA_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VGA_Color color);`

Draws a straight line between two points.

Useful for:

- aiming lines
- debug overlays
- simple effects
- wireframe shapes

## Driver Hook

### `void VGA_OnLineInterrupt(void);`

This is a driver integration hook used by the timer interrupt path.

Normal gameplay code should **not** call this directly.

It is already connected in `main.c` through:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    VGA_OnLineInterrupt();
  }
}
```

If someone rewrites startup code later, this hook still needs to remain connected for VGA output to work.

## Current Example in `main.c`

The current sample scene does the following:

- clear screen to black
- draw a green border
- draw two yellow diagonal lines
- draw one red filled box near the top-left
- draw one green filled box near the bottom-right

This example is only meant to show API usage. It is not part of the driver itself.

## Recommended Team Usage

A good split of responsibilities is:

- VGA driver owner
  Maintains `vga.c`, timings, DMA, and low-level rendering behavior
- game logic teammates
  Only call public functions from `vga.h`

Example pattern for game code:

```c
VGA_Clear(VGA_COLOR_BLACK);
VGA_FillRect(player_x, player_y, 4, 4, VGA_COLOR_GREEN);
VGA_FillRect(enemy_x, enemy_y, 4, 4, VGA_COLOR_RED);
VGA_DrawRect(0, 0, VGA_WIDTH, VGA_HEIGHT, VGA_COLOR_YELLOW);
```

## Current Limitations

- No text API yet
- No sprite API yet
- No tile map helper yet
- No double buffering yet
- Public colors are currently limited to black, red, green, and yellow

Because there is no double buffering yet, drawing while the display is actively scanning may eventually cause visible tearing depending on how game updates are scheduled.

## Suggested Next APIs

If the game team needs them, the next useful additions are:

- `VGA_DrawSprite(...)`
- `VGA_DrawText(...)`
- `VGA_DrawTileMap(...)`
- `VGA_DrawCircle(...)`
- `VGA_Present()` if a future double-buffered model is added
