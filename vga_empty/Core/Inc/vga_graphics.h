#ifndef VGA_GRAPHICS_H
#define VGA_GRAPHICS_H

#include <stdint.h>

#include "main.h"

typedef uint8_t VgaColor;

#define VGA_COLOR_BLACK  ((uint8_t)0x00U)
#define VGA_COLOR_RED    ((uint8_t)GPIO_PIN_5)
#define VGA_COLOR_GREEN  ((uint8_t)GPIO_PIN_7)
#define VGA_COLOR_YELLOW ((uint8_t)(GPIO_PIN_5 | GPIO_PIN_7))

#define VGA_H_SYNC       (128U / 8U)
#define VGA_H_BACKPORCH  (88U / 8U)
#define VGA_H_VISIBLE    (800U / 8U)
#define VGA_H_FRONTPORCH (40U / 8U)
#define VGA_H_PIXEL      (VGA_H_SYNC + VGA_H_BACKPORCH + VGA_H_VISIBLE + VGA_H_FRONTPORCH)
#define VGA_H_VISIBLE_START (VGA_H_SYNC + VGA_H_BACKPORCH)
#define VGA_H_VISIBLE_END   (VGA_H_VISIBLE_START + VGA_H_VISIBLE)

#define VGA_V_SYNC_LINES       4U
#define VGA_V_BACKPORCH_LINES  23U
#define VGA_V_VISIBLE_LINES    600U
#define VGA_V_FRONTPORCH_LINES 1U
#define VGA_V_TOTAL_LINES (VGA_V_SYNC_LINES + VGA_V_BACKPORCH_LINES + VGA_V_VISIBLE_LINES + VGA_V_FRONTPORCH_LINES)
#define VGA_V_VISIBLE_LINE_START (VGA_V_SYNC_LINES + VGA_V_BACKPORCH_LINES)
#define VGA_V_VISIBLE_LINE_END   (VGA_V_VISIBLE_LINE_START + VGA_V_VISIBLE_LINES)

void VgaGfx_Init(int16_t h_phase_offset_samples);
void VgaGfx_Clear(VgaColor color);
void VgaGfx_SetPixel(int16_t x, int16_t y, VgaColor color);
void VgaGfx_DrawHLine(int16_t x0, int16_t x1, int16_t y, VgaColor color);
void VgaGfx_DrawVLine(int16_t x, int16_t y0, int16_t y1, VgaColor color);
void VgaGfx_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VgaColor color);
void VgaGfx_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, VgaColor color);
void VgaGfx_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, VgaColor color);
void VgaGfx_DrawCircle(int16_t cx, int16_t cy, int16_t radius, VgaColor color);
void VgaGfx_FillCircle(int16_t cx, int16_t cy, int16_t radius, VgaColor color);
void VgaGfx_DrawSimpleEmoji(int16_t cx, int16_t cy, int16_t radius);

const uint32_t *VgaGfx_GetBlankLineBuffer(void);
const uint32_t *VgaGfx_GetVisibleLineBuffer(uint16_t visible_line);

#endif /* VGA_GRAPHICS_H */
