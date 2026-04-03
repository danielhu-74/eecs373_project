# VGA Graphics Interface

## Purpose

`vga_graphics` gives you a small software drawing API on top of the current VGA output path.

You can use it to:

- clear the screen
- set a pixel
- draw lines
- draw rectangles
- draw circles
- draw a simple emoji

## Current Pipeline

Yes: the current project does draw into a buffer first, and then DMA sends color data out.

The flow is:

1. Your game or demo code draws into a software framebuffer inside `vga_graphics.c`.
2. At each visible scanline, that framebuffer line is converted into a DMA scanline buffer.
3. `TIM2` DMA writes that scanline buffer to `GPIOA->BSRR`.
4. `PA5` and `PA7` become the active color outputs for that line.

So this is not direct per-pixel GPIO bit-banging from user code.  
User code writes to a framebuffer-like layer first.

## Resolution Note

The current logical drawing resolution is:

- width: `VGA_H_VISIBLE = 800 / 8 = 100`
- height: `VGA_V_VISIBLE_LINES = 600`

That means your draw APIs work in a `100 x 600` logical space.

This is because the current horizontal pipeline samples one visible point for every 8 VGA pixel clocks.

## Header

Use:

```c
#include "vga_graphics.h"
```

## Available Colors

```c
VGA_COLOR_BLACK
VGA_COLOR_RED
VGA_COLOR_GREEN
VGA_COLOR_YELLOW
```

## Main APIs

Initialization:

```c
VgaGfx_Init(0);
```

Clear:

```c
VgaGfx_Clear(VGA_COLOR_BLACK);
```

Pixel:

```c
VgaGfx_SetPixel(10, 20, VGA_COLOR_RED);
```

Horizontal and vertical lines:

```c
VgaGfx_DrawHLine(5, 80, 40, VGA_COLOR_GREEN);
VgaGfx_DrawVLine(50, 100, 180, VGA_COLOR_RED);
```

General line:

```c
VgaGfx_DrawLine(10, 10, 90, 120, VGA_COLOR_YELLOW);
```

Rectangles:

```c
VgaGfx_DrawRect(10, 20, 30, 40, VGA_COLOR_RED);
VgaGfx_FillRect(50, 60, 20, 30, VGA_COLOR_GREEN);
```

Circles:

```c
VgaGfx_DrawCircle(50, 120, 20, VGA_COLOR_GREEN);
VgaGfx_FillCircle(50, 120, 18, VGA_COLOR_YELLOW);
```

Simple emoji:

```c
VgaGfx_DrawSimpleEmoji(50, 300, 18);
```

## Example

This is a minimal example of drawing a court-like frame plus one emoji:

```c
VgaGfx_Init(0);
VgaGfx_Clear(VGA_COLOR_BLACK);

VgaGfx_DrawRect(2, 2, 96, 596, VGA_COLOR_GREEN);
VgaGfx_DrawVLine(50, 40, 560, VGA_COLOR_YELLOW);
VgaGfx_DrawSimpleEmoji(25, 220, 14);
VgaGfx_DrawSimpleEmoji(75, 220, 14);
```

## Current Main.c Usage

The project now uses the graphics interface in `VGA_InitTestPatternBuffers()` to draw a simple emoji scene before video starts.

If you want to draw your own scene, replace that code with your own `VgaGfx_*()` calls.

## Good Next Step For Your Team

For a 2D badminton game, a practical split is:

- one person writes game state and physics
- one person writes collision
- one person writes input
- one person writes draw code using `VgaGfx_*()`

The draw side should only need these APIs, without touching timer or DMA code.
