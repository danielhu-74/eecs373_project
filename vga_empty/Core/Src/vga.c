#include "vga.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern DMA_HandleTypeDef hdma_tim2_up;
extern DMA_HandleTypeDef hdma_tim4_up;

#define VGA_VSYNC_PORT GPIOB
#define VGA_VSYNC_PIN  GPIO_PIN_8
#define VGA_BLUE_PORT  GPIOD
#define VGA_BLUE_PIN   GPIO_PIN_15

#define VGA_RED_PIN    ((uint8_t)GPIO_PIN_5)
#define VGA_GREEN_PIN  ((uint8_t)GPIO_PIN_7)
#define HSYNC_PULSE_TICKS 384U

#define H_SYNC       (128U / 8U)
#define H_BACKPORCH  (88U / 8U)
#define H_VISIBLE    VGA_WIDTH
#define H_FRONTPORCH (40U / 8U)
#define H_PIXEL      (H_SYNC + H_BACKPORCH + H_VISIBLE + H_FRONTPORCH)

#define V_SYNC_LINES       4U
#define V_BACKPORCH_LINES  23U
#define V_VISIBLE_LINES    600U
#define V_FRONTPORCH_LINES 1U
#define V_TOTAL_LINES      (V_SYNC_LINES + V_BACKPORCH_LINES + V_VISIBLE_LINES + V_FRONTPORCH_LINES)
#define V_VISIBLE_LINE_START (V_SYNC_LINES + V_BACKPORCH_LINES)
#define V_VISIBLE_LINE_END   (V_VISIBLE_LINE_START + V_VISIBLE_LINES)

#define VGA_VERTICAL_SCALE 8U
#define H_PHASE_OFFSET_SAMPLES (-2)

static uint32_t blank_line_buffer[H_PIXEL];
static uint32_t frame_buffer[VGA_HEIGHT][H_PIXEL];
static const uint32_t *active_line_src = 0;
static uint32_t tim2_cnt_reset_word = 0U;
static uint16_t current_line = 0U;
static uint16_t visible_start_sample = 0U;

static uint32_t VGA_ColorToBsrr(VGA_Color color)
{
  uint8_t gpio_color = 0U;
  uint32_t set_mask;
  uint32_t reset_mask;

  if ((color == VGA_COLOR_RED) || (color == VGA_COLOR_YELLOW))
  {
    gpio_color |= VGA_RED_PIN;
  }
  if ((color == VGA_COLOR_GREEN) || (color == VGA_COLOR_YELLOW))
  {
    gpio_color |= VGA_GREEN_PIN;
  }

  set_mask = (uint32_t)gpio_color;
  reset_mask = (uint32_t)((VGA_RED_PIN | VGA_GREEN_PIN) & (uint8_t)(~gpio_color));
  return set_mask | (reset_mask << 16U);
}

static uint16_t VGA_ComputeVisibleStart(void)
{
  int32_t visible_start_i;

  visible_start_i = (int32_t)(H_SYNC + H_BACKPORCH) + (int32_t)H_PHASE_OFFSET_SAMPLES;
  if (visible_start_i < (int32_t)H_SYNC)
  {
    visible_start_i = (int32_t)H_SYNC;
  }
  if (visible_start_i > (int32_t)(H_PIXEL - H_VISIBLE))
  {
    visible_start_i = (int32_t)(H_PIXEL - H_VISIBLE);
  }

  return (uint16_t)visible_start_i;
}

static void VGA_ResetRow(uint16_t row, uint32_t encoded_color)
{
  uint16_t h;
  uint16_t visible_end;

  visible_end = (uint16_t)(visible_start_sample + H_VISIBLE);

  for (h = 0; h < H_PIXEL; ++h)
  {
    frame_buffer[row][h] = blank_line_buffer[h];
  }

  for (h = visible_start_sample; h < visible_end; ++h)
  {
    frame_buffer[row][h] = encoded_color;
  }
}

static void VGA_SetPixelUnchecked(uint16_t x, uint16_t y, uint32_t encoded_color)
{
  frame_buffer[y][visible_start_sample + x] = encoded_color;
}

void VGA_Init(void)
{
  uint16_t h;
  uint16_t y;
  uint32_t encoded_black;

  visible_start_sample = VGA_ComputeVisibleStart();
  encoded_black = VGA_ColorToBsrr(VGA_COLOR_BLACK);

  for (h = 0; h < H_PIXEL; ++h)
  {
    blank_line_buffer[h] = encoded_black;
  }

  for (y = 0; y < VGA_HEIGHT; ++y)
  {
    VGA_ResetRow(y, encoded_black);
  }

  active_line_src = blank_line_buffer;
}

void VGA_Start(void)
{
  current_line = (uint16_t)(V_TOTAL_LINES - 1U);
  VGA_VSYNC_PORT->BSRR = (uint32_t)VGA_VSYNC_PIN << 16U;
  VGA_BLUE_PORT->BSRR = (uint32_t)VGA_BLUE_PIN << 16U;
  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, HSYNC_PULSE_TICKS);

  active_line_src = blank_line_buffer;
  if (HAL_DMA_Start(&hdma_tim4_up, (uint32_t)&tim2_cnt_reset_word, (uint32_t)&TIM2->CNT, 1U) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_DMA_Start(&hdma_tim2_up, (uint32_t)active_line_src, (uint32_t)&GPIOA->BSRR, H_PIXEL) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);

  if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
}

void VGA_Clear(VGA_Color color)
{
  uint16_t y;
  uint32_t encoded_color;

  encoded_color = VGA_ColorToBsrr(color);
  for (y = 0; y < VGA_HEIGHT; ++y)
  {
    VGA_ResetRow(y, encoded_color);
  }
}

void VGA_DrawPixel(int16_t x, int16_t y, VGA_Color color)
{
  if ((x < 0) || (y < 0) || (x >= (int16_t)VGA_WIDTH) || (y >= (int16_t)VGA_HEIGHT))
  {
    return;
  }

  VGA_SetPixelUnchecked((uint16_t)x, (uint16_t)y, VGA_ColorToBsrr(color));
}

void VGA_DrawHLine(int16_t x, int16_t y, int16_t length, VGA_Color color)
{
  int16_t x_end;
  int16_t draw_x;
  uint32_t encoded_color;

  if ((y < 0) || (y >= (int16_t)VGA_HEIGHT) || (length <= 0))
  {
    return;
  }

  x_end = (int16_t)(x + length);
  if ((x_end <= 0) || (x >= (int16_t)VGA_WIDTH))
  {
    return;
  }

  if (x < 0)
  {
    x = 0;
  }
  if (x_end > (int16_t)VGA_WIDTH)
  {
    x_end = (int16_t)VGA_WIDTH;
  }

  encoded_color = VGA_ColorToBsrr(color);
  for (draw_x = x; draw_x < x_end; ++draw_x)
  {
    VGA_SetPixelUnchecked((uint16_t)draw_x, (uint16_t)y, encoded_color);
  }
}

void VGA_DrawVLine(int16_t x, int16_t y, int16_t length, VGA_Color color)
{
  int16_t y_end;
  int16_t draw_y;
  uint32_t encoded_color;

  if ((x < 0) || (x >= (int16_t)VGA_WIDTH) || (length <= 0))
  {
    return;
  }

  y_end = (int16_t)(y + length);
  if ((y_end <= 0) || (y >= (int16_t)VGA_HEIGHT))
  {
    return;
  }

  if (y < 0)
  {
    y = 0;
  }
  if (y_end > (int16_t)VGA_HEIGHT)
  {
    y_end = (int16_t)VGA_HEIGHT;
  }

  encoded_color = VGA_ColorToBsrr(color);
  for (draw_y = y; draw_y < y_end; ++draw_y)
  {
    VGA_SetPixelUnchecked((uint16_t)x, (uint16_t)draw_y, encoded_color);
  }
}

void VGA_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color)
{
  if ((width <= 0) || (height <= 0))
  {
    return;
  }

  VGA_DrawHLine(x, y, width, color);
  VGA_DrawHLine(x, (int16_t)(y + height - 1), width, color);
  VGA_DrawVLine(x, y, height, color);
  VGA_DrawVLine((int16_t)(x + width - 1), y, height, color);
}

void VGA_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, VGA_Color color)
{
  int16_t x_end;
  int16_t y_end;
  int16_t draw_x;
  int16_t draw_y;
  uint32_t encoded_color;

  if ((width <= 0) || (height <= 0))
  {
    return;
  }

  x_end = (int16_t)(x + width);
  y_end = (int16_t)(y + height);
  if ((x_end <= 0) || (y_end <= 0) || (x >= (int16_t)VGA_WIDTH) || (y >= (int16_t)VGA_HEIGHT))
  {
    return;
  }

  if (x < 0)
  {
    x = 0;
  }
  if (y < 0)
  {
    y = 0;
  }
  if (x_end > (int16_t)VGA_WIDTH)
  {
    x_end = (int16_t)VGA_WIDTH;
  }
  if (y_end > (int16_t)VGA_HEIGHT)
  {
    y_end = (int16_t)VGA_HEIGHT;
  }

  encoded_color = VGA_ColorToBsrr(color);
  for (draw_y = y; draw_y < y_end; ++draw_y)
  {
    for (draw_x = x; draw_x < x_end; ++draw_x)
    {
      VGA_SetPixelUnchecked((uint16_t)draw_x, (uint16_t)draw_y, encoded_color);
    }
  }
}

void VGA_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VGA_Color color)
{
  int16_t dx;
  int16_t sx;
  int16_t dy;
  int16_t sy;
  int16_t err;
  int16_t e2;

  dx = (x0 < x1) ? (int16_t)(x1 - x0) : (int16_t)(x0 - x1);
  sx = (x0 < x1) ? 1 : -1;
  dy = (y0 < y1) ? (int16_t)(y0 - y1) : (int16_t)(y1 - y0);
  sy = (y0 < y1) ? 1 : -1;
  err = (int16_t)(dx + dy);

  while (1)
  {
    VGA_DrawPixel(x0, y0, color);
    if ((x0 == x1) && (y0 == y1))
    {
      break;
    }

    e2 = (int16_t)(2 * err);
    if (e2 >= dy)
    {
      err = (int16_t)(err + dy);
      x0 = (int16_t)(x0 + sx);
    }
    if (e2 <= dx)
    {
      err = (int16_t)(err + dx);
      y0 = (int16_t)(y0 + sy);
    }
  }
}

void VGA_OnLineInterrupt(void)
{
  const uint32_t *next_line_src;

  current_line++;
  if (current_line >= V_TOTAL_LINES)
  {
    current_line = 0U;
  }

  if (current_line == 0U)
  {
    VGA_VSYNC_PORT->BSRR = VGA_VSYNC_PIN;
  }
  else if (current_line == V_SYNC_LINES)
  {
    VGA_VSYNC_PORT->BSRR = (uint32_t)VGA_VSYNC_PIN << 16U;
  }

  if ((current_line >= V_VISIBLE_LINE_START) && (current_line < V_VISIBLE_LINE_END))
  {
    uint16_t row;

    row = (uint16_t)((current_line - V_VISIBLE_LINE_START) / VGA_VERTICAL_SCALE);
    next_line_src = frame_buffer[row];
  }
  else
  {
    next_line_src = blank_line_buffer;
  }

  if (active_line_src != next_line_src)
  {
    active_line_src = next_line_src;
    hdma_tim2_up.Instance->CMAR = (uint32_t)active_line_src;
  }
}
