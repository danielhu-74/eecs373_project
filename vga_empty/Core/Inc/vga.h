#ifndef __VGA_H
#define __VGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define VGA_WIDTH  100U
#define VGA_HEIGHT 75U

typedef enum
{
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_RED,
  VGA_COLOR_GREEN,
  VGA_COLOR_YELLOW
} VGA_Color;

void VGA_Init(void);
void VGA_Start(void);
void VGA_Clear(VGA_Color color);
void VGA_DrawPixel(int16_t x, int16_t y, VGA_Color color);
void VGA_DrawHLine(int16_t x, int16_t y, int16_t length, VGA_Color color);
void VGA_DrawVLine(int16_t x, int16_t y, int16_t length, VGA_Color color);
void VGA_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color);
void VGA_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color);
void VGA_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VGA_Color color);

/* Driver integration hook: call from the TIM4 period-elapsed callback. */
void VGA_OnLineInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __VGA_H */
