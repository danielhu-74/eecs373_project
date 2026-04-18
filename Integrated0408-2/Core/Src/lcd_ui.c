#include "lcd_ui.h"

#include <stddef.h>
#include <string.h>

#include "lcd_minimal.h"
#include "touch_xpt2046.h"

#define LCD_UI_GLYPH_WIDTH   5U
#define LCD_UI_GLYPH_HEIGHT  7U

static uint8_t lcd_ui_ready = 0U;

static const uint8_t font_space[7] = {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};

static const uint8_t font_bang[7] = {
    0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x00U, 0x04U
};

static const uint8_t font_apostrophe[7] = {
    0x04U, 0x04U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U
};

static const uint8_t font_colon[7] = {
    0x00U, 0x04U, 0x04U, 0x00U, 0x04U, 0x04U, 0x00U
};

static const uint8_t font_dash[7] = {
    0x00U, 0x00U, 0x00U, 0x1FU, 0x00U, 0x00U, 0x00U
};

static const uint8_t font_0[7] = {
    0x0EU, 0x11U, 0x13U, 0x15U, 0x19U, 0x11U, 0x0EU
};
static const uint8_t font_1[7] = {
    0x04U, 0x0CU, 0x04U, 0x04U, 0x04U, 0x04U, 0x0EU
};
static const uint8_t font_2[7] = {
    0x0EU, 0x11U, 0x01U, 0x02U, 0x04U, 0x08U, 0x1FU
};
static const uint8_t font_3[7] = {
    0x1EU, 0x01U, 0x01U, 0x0EU, 0x01U, 0x01U, 0x1EU
};
static const uint8_t font_4[7] = {
    0x02U, 0x06U, 0x0AU, 0x12U, 0x1FU, 0x02U, 0x02U
};
static const uint8_t font_5[7] = {
    0x1FU, 0x10U, 0x10U, 0x1EU, 0x01U, 0x01U, 0x1EU
};
static const uint8_t font_6[7] = {
    0x0EU, 0x10U, 0x10U, 0x1EU, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_7[7] = {
    0x1FU, 0x01U, 0x02U, 0x04U, 0x08U, 0x08U, 0x08U
};
static const uint8_t font_8[7] = {
    0x0EU, 0x11U, 0x11U, 0x0EU, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_9[7] = {
    0x0EU, 0x11U, 0x11U, 0x0FU, 0x01U, 0x01U, 0x0EU
};

static const uint8_t font_A[7] = {
    0x0EU, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_C[7] = {
    0x0FU, 0x10U, 0x10U, 0x10U, 0x10U, 0x10U, 0x0FU
};
static const uint8_t font_D[7] = {
    0x1EU, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x1EU
};
static const uint8_t font_E[7] = {
    0x1FU, 0x10U, 0x10U, 0x1EU, 0x10U, 0x10U, 0x1FU
};
static const uint8_t font_G[7] = {
    0x0EU, 0x11U, 0x10U, 0x17U, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_H[7] = {
    0x11U, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_I[7] = {
    0x0EU, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x0EU
};
static const uint8_t font_L[7] = {
    0x10U, 0x10U, 0x10U, 0x10U, 0x10U, 0x10U, 0x1FU
};
static const uint8_t font_M[7] = {
    0x11U, 0x1BU, 0x15U, 0x15U, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_N[7] = {
    0x11U, 0x19U, 0x15U, 0x13U, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_O[7] = {
    0x0EU, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_P[7] = {
    0x1EU, 0x11U, 0x11U, 0x1EU, 0x10U, 0x10U, 0x10U
};
static const uint8_t font_Q[7] = {
    0x0EU, 0x11U, 0x11U, 0x11U, 0x15U, 0x12U, 0x0DU
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
static const uint8_t font_U[7] = {
    0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0EU
};
static const uint8_t font_V[7] = {
    0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0AU, 0x04U
};
static const uint8_t font_W[7] = {
    0x11U, 0x11U, 0x11U, 0x15U, 0x15U, 0x1BU, 0x11U
};
static const uint8_t font_Y[7] = {
    0x11U, 0x11U, 0x0AU, 0x04U, 0x04U, 0x04U, 0x04U
};

static const uint8_t *lcd_ui_get_glyph(char c)
{
    switch (c) {
    case '!':
        return font_bang;
    case '\'':
        return font_apostrophe;
    case ':':
        return font_colon;
    case '-':
        return font_dash;
    case '0':
        return font_0;
    case '1':
        return font_1;
    case '2':
        return font_2;
    case '3':
        return font_3;
    case '4':
        return font_4;
    case '5':
        return font_5;
    case '6':
        return font_6;
    case '7':
        return font_7;
    case '8':
        return font_8;
    case '9':
        return font_9;
    case 'A':
        return font_A;
    case 'C':
        return font_C;
    case 'D':
        return font_D;
    case 'E':
        return font_E;
    case 'G':
        return font_G;
    case 'H':
        return font_H;
    case 'I':
        return font_I;
    case 'L':
        return font_L;
    case 'M':
        return font_M;
    case 'N':
        return font_N;
    case 'O':
        return font_O;
    case 'P':
        return font_P;
    case 'Q':
        return font_Q;
    case 'R':
        return font_R;
    case 'S':
        return font_S;
    case 'T':
        return font_T;
    case 'U':
        return font_U;
    case 'V':
        return font_V;
    case 'W':
        return font_W;
    case 'Y':
        return font_Y;
    case ' ':
    default:
        return font_space;
    }
}

static HAL_StatusTypeDef lcd_ui_draw_char(uint16_t x,
                                          uint16_t y,
                                          char c,
                                          uint16_t scale,
                                          uint8_t red,
                                          uint8_t green,
                                          uint8_t blue)
{
    const uint8_t *glyph;
    uint8_t row;
    uint8_t col;

    if (scale == 0U) {
        return HAL_ERROR;
    }

    glyph = lcd_ui_get_glyph(c);

    for (row = 0U; row < LCD_UI_GLYPH_HEIGHT; ++row) {
        for (col = 0U; col < LCD_UI_GLYPH_WIDTH; ++col) {
            if ((glyph[row] & (uint8_t)(1U << (LCD_UI_GLYPH_WIDTH - 1U - col))) == 0U) {
                continue;
            }

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

    return HAL_OK;
}

HAL_StatusTypeDef LCD_UI_EnsureReady(void)
{
    if (lcd_ui_ready != 0U) {
        return HAL_OK;
    }

    if (LCD_Minimal_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (Touch_XPT2046_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    lcd_ui_ready = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef LCD_UI_Clear(uint8_t red, uint8_t green, uint8_t blue)
{
    if (LCD_UI_EnsureReady() != HAL_OK) {
        return HAL_ERROR;
    }

    return LCD_Minimal_FillColor(red, green, blue);
}

HAL_StatusTypeDef LCD_UI_DrawText(uint16_t x,
                                  uint16_t y,
                                  const char *text,
                                  uint16_t scale,
                                  uint8_t red,
                                  uint8_t green,
                                  uint8_t blue)
{
    size_t i;
    uint16_t char_width;
    uint16_t char_gap;

    if (text == NULL || scale == 0U) {
        return HAL_ERROR;
    }

    if (LCD_UI_EnsureReady() != HAL_OK) {
        return HAL_ERROR;
    }

    char_width = (uint16_t)(LCD_UI_GLYPH_WIDTH * scale);
    char_gap = scale;

    for (i = 0U; i < strlen(text); ++i) {
        if (lcd_ui_draw_char((uint16_t)(x + (uint16_t)i * (char_width + char_gap)),
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

HAL_StatusTypeDef LCD_UI_DrawTextCentered(uint16_t center_x,
                                          uint16_t y,
                                          const char *text,
                                          uint16_t scale,
                                          uint8_t red,
                                          uint8_t green,
                                          uint8_t blue)
{
    uint16_t width;
    uint16_t start_x;

    if (text == NULL) {
        return HAL_ERROR;
    }

    width = LCD_UI_GetTextWidth(text, scale);
    start_x = (center_x > (width / 2U)) ? (uint16_t)(center_x - (width / 2U)) : 0U;

    return LCD_UI_DrawText(start_x, y, text, scale, red, green, blue);
}

uint16_t LCD_UI_GetTextWidth(const char *text, uint16_t scale)
{
    size_t len;

    if (text == NULL || scale == 0U) {
        return 0U;
    }

    len = strlen(text);

    if (len == 0U) {
        return 0U;
    }

    return (uint16_t)(len * (LCD_UI_GLYPH_WIDTH * scale) + (len - 1U) * scale);
}

uint16_t LCD_UI_GetTextHeight(uint16_t scale)
{
    if (scale == 0U) {
        return 0U;
    }

    return (uint16_t)(LCD_UI_GLYPH_HEIGHT * scale);
}
