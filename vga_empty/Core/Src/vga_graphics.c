#include "vga_graphics.h"

static uint8_t vga_framebuffer[VGA_V_VISIBLE_LINES][VGA_H_VISIBLE];
static uint32_t vga_blank_line_buffer[VGA_H_PIXEL];
static uint32_t vga_visible_line_buffers[2][VGA_H_PIXEL];
static uint16_t vga_visible_start = VGA_H_VISIBLE_START;

static uint32_t VgaGfx_ColorToBsrr(VgaColor color)
{
  uint32_t set_mask;
  uint32_t reset_mask;

  set_mask = (uint32_t)(color & (VGA_COLOR_RED | VGA_COLOR_GREEN));
  reset_mask = (uint32_t)((VGA_COLOR_RED | VGA_COLOR_GREEN) & (uint8_t)(~color));
  return set_mask | (reset_mask << 16U);
}

static int16_t VgaGfx_Abs(int16_t value)
{
  return (value < 0) ? (int16_t)(-value) : value;
}

void VgaGfx_Init(int16_t h_phase_offset_samples)
{
  int32_t visible_start_i;
  uint16_t h;

  visible_start_i = (int32_t)VGA_H_VISIBLE_START + (int32_t)h_phase_offset_samples;
  if (visible_start_i < (int32_t)VGA_H_SYNC)
  {
    visible_start_i = (int32_t)VGA_H_SYNC;
  }
  if (visible_start_i > (int32_t)(VGA_H_PIXEL - VGA_H_VISIBLE))
  {
    visible_start_i = (int32_t)(VGA_H_PIXEL - VGA_H_VISIBLE);
  }

  vga_visible_start = (uint16_t)visible_start_i;

  for (h = 0; h < VGA_H_PIXEL; ++h)
  {
    vga_blank_line_buffer[h] = VgaGfx_ColorToBsrr(VGA_COLOR_BLACK);
    vga_visible_line_buffers[0][h] = VgaGfx_ColorToBsrr(VGA_COLOR_BLACK);
    vga_visible_line_buffers[1][h] = VgaGfx_ColorToBsrr(VGA_COLOR_BLACK);
  }

  VgaGfx_Clear(VGA_COLOR_BLACK);
}

void VgaGfx_Clear(VgaColor color)
{
  uint16_t x;
  uint16_t y;

  for (y = 0; y < VGA_V_VISIBLE_LINES; ++y)
  {
    for (x = 0; x < VGA_H_VISIBLE; ++x)
    {
      vga_framebuffer[y][x] = color;
    }
  }
}

void VgaGfx_SetPixel(int16_t x, int16_t y, VgaColor color)
{
  if ((x < 0) || (y < 0) || (x >= (int16_t)VGA_H_VISIBLE) || (y >= (int16_t)VGA_V_VISIBLE_LINES))
  {
    return;
  }

  vga_framebuffer[y][x] = color;
}

void VgaGfx_DrawHLine(int16_t x0, int16_t x1, int16_t y, VgaColor color)
{
  int16_t x;
  int16_t start;
  int16_t end;

  if ((y < 0) || (y >= (int16_t)VGA_V_VISIBLE_LINES))
  {
    return;
  }

  start = (x0 < x1) ? x0 : x1;
  end = (x0 < x1) ? x1 : x0;

  for (x = start; x <= end; ++x)
  {
    VgaGfx_SetPixel(x, y, color);
  }
}

void VgaGfx_DrawVLine(int16_t x, int16_t y0, int16_t y1, VgaColor color)
{
  int16_t y;
  int16_t start;
  int16_t end;

  if ((x < 0) || (x >= (int16_t)VGA_H_VISIBLE))
  {
    return;
  }

  start = (y0 < y1) ? y0 : y1;
  end = (y0 < y1) ? y1 : y0;

  for (y = start; y <= end; ++y)
  {
    VgaGfx_SetPixel(x, y, color);
  }
}

void VgaGfx_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VgaColor color)
{
  int16_t dx;
  int16_t sx;
  int16_t dy;
  int16_t sy;
  int16_t err;

  dx = VgaGfx_Abs((int16_t)(x1 - x0));
  sx = (x0 < x1) ? 1 : -1;
  dy = (int16_t)(-VgaGfx_Abs((int16_t)(y1 - y0)));
  sy = (y0 < y1) ? 1 : -1;
  err = (int16_t)(dx + dy);

  while (1)
  {
    VgaGfx_SetPixel(x0, y0, color);
    if ((x0 == x1) && (y0 == y1))
    {
      break;
    }

    if ((int16_t)(2 * err) >= dy)
    {
      err = (int16_t)(err + dy);
      x0 = (int16_t)(x0 + sx);
    }
    if ((int16_t)(2 * err) <= dx)
    {
      err = (int16_t)(err + dx);
      y0 = (int16_t)(y0 + sy);
    }
  }
}

void VgaGfx_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, VgaColor color)
{
  if ((w <= 0) || (h <= 0))
  {
    return;
  }

  VgaGfx_DrawHLine(x, (int16_t)(x + w - 1), y, color);
  VgaGfx_DrawHLine(x, (int16_t)(x + w - 1), (int16_t)(y + h - 1), color);
  VgaGfx_DrawVLine(x, y, (int16_t)(y + h - 1), color);
  VgaGfx_DrawVLine((int16_t)(x + w - 1), y, (int16_t)(y + h - 1), color);
}

void VgaGfx_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, VgaColor color)
{
  int16_t yy;

  if ((w <= 0) || (h <= 0))
  {
    return;
  }

  for (yy = y; yy < (int16_t)(y + h); ++yy)
  {
    VgaGfx_DrawHLine(x, (int16_t)(x + w - 1), yy, color);
  }
}

void VgaGfx_DrawCircle(int16_t cx, int16_t cy, int16_t radius, VgaColor color)
{
  int16_t x;
  int16_t y;
  int16_t d;

  if (radius <= 0)
  {
    return;
  }

  x = radius;
  y = 0;
  d = 1 - radius;

  while (y <= x)
  {
    VgaGfx_SetPixel((int16_t)(cx + x), (int16_t)(cy + y), color);
    VgaGfx_SetPixel((int16_t)(cx + y), (int16_t)(cy + x), color);
    VgaGfx_SetPixel((int16_t)(cx - y), (int16_t)(cy + x), color);
    VgaGfx_SetPixel((int16_t)(cx - x), (int16_t)(cy + y), color);
    VgaGfx_SetPixel((int16_t)(cx - x), (int16_t)(cy - y), color);
    VgaGfx_SetPixel((int16_t)(cx - y), (int16_t)(cy - x), color);
    VgaGfx_SetPixel((int16_t)(cx + y), (int16_t)(cy - x), color);
    VgaGfx_SetPixel((int16_t)(cx + x), (int16_t)(cy - y), color);

    ++y;
    if (d < 0)
    {
      d = (int16_t)(d + (2 * y) + 1);
    }
    else
    {
      --x;
      d = (int16_t)(d + (2 * (y - x)) + 1);
    }
  }
}

void VgaGfx_FillCircle(int16_t cx, int16_t cy, int16_t radius, VgaColor color)
{
  int16_t y;
  int16_t x_limit;

  if (radius <= 0)
  {
    return;
  }

  for (y = -radius; y <= radius; ++y)
  {
    int32_t rr = (int32_t)radius * (int32_t)radius;
    int32_t yy = (int32_t)y * (int32_t)y;
    int16_t x = 0;

    while (((int32_t)x * (int32_t)x) <= (rr - yy))
    {
      ++x;
    }

    x_limit = (int16_t)(x - 1);
    VgaGfx_DrawHLine((int16_t)(cx - x_limit), (int16_t)(cx + x_limit), (int16_t)(cy + y), color);
  }
}

void VgaGfx_DrawSimpleEmoji(int16_t cx, int16_t cy, int16_t radius)
{
  int16_t eye_offset_x;
  int16_t eye_offset_y;
  int16_t eye_radius;
  int16_t mouth_y;
  int16_t mouth_half_width;

  if (radius < 6)
  {
    radius = 6;
  }

  eye_offset_x = (int16_t)(radius / 3);
  eye_offset_y = (int16_t)(radius / 4);
  eye_radius = (int16_t)(radius / 8);
  if (eye_radius < 1)
  {
    eye_radius = 1;
  }

  mouth_y = (int16_t)(cy + (radius / 4));
  mouth_half_width = (int16_t)(radius / 2);

  VgaGfx_FillCircle(cx, cy, radius, VGA_COLOR_YELLOW);
  VgaGfx_DrawCircle(cx, cy, radius, VGA_COLOR_GREEN);

  VgaGfx_FillCircle((int16_t)(cx - eye_offset_x), (int16_t)(cy - eye_offset_y), eye_radius, VGA_COLOR_BLACK);
  VgaGfx_FillCircle((int16_t)(cx + eye_offset_x), (int16_t)(cy - eye_offset_y), eye_radius, VGA_COLOR_BLACK);

  VgaGfx_DrawLine((int16_t)(cx - mouth_half_width), mouth_y, (int16_t)(cx - (radius / 4)), (int16_t)(mouth_y + (radius / 5)), VGA_COLOR_BLACK);
  VgaGfx_DrawLine((int16_t)(cx - (radius / 4)), (int16_t)(mouth_y + (radius / 5)), (int16_t)(cx + (radius / 4)), (int16_t)(mouth_y + (radius / 5)), VGA_COLOR_BLACK);
  VgaGfx_DrawLine((int16_t)(cx + (radius / 4)), (int16_t)(mouth_y + (radius / 5)), (int16_t)(cx + mouth_half_width), mouth_y, VGA_COLOR_BLACK);
}

const uint32_t *VgaGfx_GetBlankLineBuffer(void)
{
  return vga_blank_line_buffer;
}

const uint32_t *VgaGfx_GetVisibleLineBuffer(uint16_t visible_line)
{
  uint16_t h;
  uint16_t buffer_index;
  uint16_t visible_end;
  uint32_t *line_buffer;

  if (visible_line >= VGA_V_VISIBLE_LINES)
  {
    return vga_blank_line_buffer;
  }

  buffer_index = (uint16_t)(visible_line & 1U);
  visible_end = (uint16_t)(vga_visible_start + VGA_H_VISIBLE);
  line_buffer = vga_visible_line_buffers[buffer_index];

  for (h = 0; h < VGA_H_PIXEL; ++h)
  {
    line_buffer[h] = VgaGfx_ColorToBsrr(VGA_COLOR_BLACK);
  }

  for (h = vga_visible_start; h < visible_end; ++h)
  {
    uint16_t x = (uint16_t)(h - vga_visible_start);
    line_buffer[h] = VgaGfx_ColorToBsrr(vga_framebuffer[visible_line][x]);
  }

  return line_buffer;
}
