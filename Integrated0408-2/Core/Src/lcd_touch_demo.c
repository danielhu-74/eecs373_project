#include "lcd_touch_demo.h"

#include <stdio.h>
#include <string.h>

#include "lcd_minimal.h"
#include "touch_xpt2046.h"

#define SPLASH_BG_R          0x00U
#define SPLASH_BG_G          0x00U
#define SPLASH_BG_B          0x00U
#define SPLASH_TEXT_R        0xFFU
#define SPLASH_TEXT_G        0xFFU
#define SPLASH_TEXT_B        0xFFU
#define SPLASH_TEXT_SCALE    6U
#define SPLASH_POLL_MS       20U

typedef struct
{
    uint8_t initialized;
    uint8_t touch_latched;
    uint32_t last_poll_tick;
} LCD_TouchDemoContext;

static const uint8_t font_space[7] = {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};

static const uint8_t font_A[7] = {
    0x0EU, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_O[7] = {
    0x0EU, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_P[7] = {
    0x1EU, 0x11U, 0x11U, 0x1EU, 0x10U, 0x10U, 0x10U
};
static const uint8_t font_R[7] = {
    0x1EU, 0x11U, 0x11U, 0x1EU, 0x14U, 0x12U, 0x11U
};
static const uint8_t font_S[7] = {
    0x0FU, 0x10U, 0x10U, 0x0EU, 0x01U, 0x01U, 0x1EU
};
static const uint8_t font_T[7] = {
    0x1FU, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U
};

static LCD_TouchDemoContext g_lcd_touch_demo = {0};

static const uint8_t *lcd_touch_demo_get_glyph(char c)
{
    switch (c) {
    case 'A':
        return font_A;
    case 'O':
        return font_O;
    case 'P':
        return font_P;
    case 'R':
        return font_R;
    case 'S':
        return font_S;
    case 'T':
        return font_T;
    case ' ':
    default:
        return font_space;
    }
}

static HAL_StatusTypeDef lcd_touch_demo_draw_char(uint16_t x,
                                                  uint16_t y,
                                                  char c,
                                                  uint16_t scale,
                                                  uint8_t red,
                                                  uint8_t green,
                                                  uint8_t blue)
{
    const uint8_t *glyph = lcd_touch_demo_get_glyph(c);
    uint8_t row;
    uint8_t col;

    for (row = 0U; row < 7U; ++row) {
        for (col = 0U; col < 5U; ++col) {
            if ((glyph[row] & (uint8_t)(1U << (4U - col))) != 0U) {
                if (LCD_Minimal_FillRect((uint16_t)(x + (uint16_t)col * scale),
                                         (uint16_t)(y + (uint16_t)row * scale),
                                         scale,
                                         scale,
                                         red,
                                         green,
                                         blue) != HAL_OK) {
                    return HAL_ERROR;
                }
            }
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef lcd_touch_demo_draw_text_centered(uint16_t center_x,
                                                           uint16_t y,
                                                           const char *text,
                                                           uint16_t scale,
                                                           uint8_t red,
                                                           uint8_t green,
                                                           uint8_t blue)
{
    size_t len = strlen(text);
    uint16_t char_width = (uint16_t)(5U * scale);
    uint16_t char_gap = scale;
    uint16_t total_width;
    uint16_t start_x;
    size_t i;

    if (len == 0U) {
        return HAL_OK;
    }

    total_width = (uint16_t)(len * char_width + (len - 1U) * char_gap);
    start_x = (center_x > (total_width / 2U)) ? (uint16_t)(center_x - (total_width / 2U)) : 0U;

    for (i = 0U; i < len; ++i) {
        if (lcd_touch_demo_draw_char((uint16_t)(start_x + (uint16_t)i * (char_width + char_gap)),
                                     y,
                                     text[i],
                                     scale,
                                     red,
                                     green,
                                     blue) != HAL_OK) {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef lcd_touch_demo_draw_splash(void)
{
    uint16_t text_height = (uint16_t)(7U * SPLASH_TEXT_SCALE);
    uint16_t text_y = (uint16_t)((LCD_MINIMAL_HEIGHT > text_height)
                                     ? ((LCD_MINIMAL_HEIGHT - text_height) / 2U)
                                     : 0U);

    if (LCD_Minimal_FillColor(SPLASH_BG_R, SPLASH_BG_G, SPLASH_BG_B) != HAL_OK) {
        return HAL_ERROR;
    }

    return lcd_touch_demo_draw_text_centered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                             text_y,
                                             "TAP TO START",
                                             SPLASH_TEXT_SCALE,
                                             SPLASH_TEXT_R,
                                             SPLASH_TEXT_G,
                                             SPLASH_TEXT_B);
}

HAL_StatusTypeDef LCD_Touch_ColorDemo_Init(void)
{
    if (LCD_Minimal_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (Touch_XPT2046_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (lcd_touch_demo_draw_splash() != HAL_OK) {
        return HAL_ERROR;
    }

    g_lcd_touch_demo.initialized = 1U;
    g_lcd_touch_demo.touch_latched = 0U;
    g_lcd_touch_demo.last_poll_tick = 0U;

    printf("Tap-to-start screen ready.\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef LCD_Touch_ColorDemo_Process(void)
{
    Touch_XPT2046_State touch_state;
    uint32_t now = HAL_GetTick();

    if (g_lcd_touch_demo.initialized == 0U) {
        return HAL_OK;
    }

    if ((now - g_lcd_touch_demo.last_poll_tick) < SPLASH_POLL_MS) {
        return HAL_OK;
    }

    g_lcd_touch_demo.last_poll_tick = now;

    if (Touch_XPT2046_ReadState(&touch_state) != HAL_OK) {
        return HAL_ERROR;
    }

    if (touch_state.valid != 0U) {
        if (g_lcd_touch_demo.touch_latched == 0U) {
            g_lcd_touch_demo.touch_latched = 1U;
            printf("Tap detected.\r\n");
        }
    } else {
        g_lcd_touch_demo.touch_latched = 0U;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LCD_Touch_ColorDemo_Run(void)
{
    if (LCD_Touch_ColorDemo_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    while (1) {
        if (LCD_Touch_ColorDemo_Process() != HAL_OK) {
            return HAL_ERROR;
        }
    }
}
