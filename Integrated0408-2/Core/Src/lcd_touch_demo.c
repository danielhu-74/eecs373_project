#include "lcd_touch_demo.h"

#include <stdio.h>
#include <string.h>

#include "lcd_minimal.h"
#include "touch_xpt2046.h"

#define UI_BG_R 0xF5U
#define UI_BG_G 0xEEU
#define UI_BG_B 0xDEU

#define UI_TITLE_R 0x1BU
#define UI_TITLE_G 0x2AU
#define UI_TITLE_B 0x41U

#define UI_ACCENT_R 0xE7U
#define UI_ACCENT_G 0x6FU
#define UI_ACCENT_B 0x51U

#define UI_CARD_R 0xFFU
#define UI_CARD_G 0xF8U
#define UI_CARD_B 0xE8U

#define UI_BUTTON1_R 0xE9U
#define UI_BUTTON1_G 0x7CU
#define UI_BUTTON1_B 0x5FU

#define UI_BUTTON2_R 0x4DU
#define UI_BUTTON2_G 0x76U
#define UI_BUTTON2_B 0xA8U

#define UI_BUTTON_ACTIVE_R 0x1FU
#define UI_BUTTON_ACTIVE_G 0xB9U
#define UI_BUTTON_ACTIVE_B 0x8BU

#define UI_TEXT_LIGHT_R 0xFFU
#define UI_TEXT_LIGHT_G 0xFAU
#define UI_TEXT_LIGHT_B 0xF2U

#define UI_BUTTON_WIDTH 240U
#define UI_BUTTON_HEIGHT 62U
#define UI_BUTTON_GAP 18U
#define UI_TITLE_SCALE 8U
#define UI_BUTTON_TEXT_SCALE 5U

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    const char *label;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} MenuButton;

static const uint8_t font_space[7] = {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};

static const uint8_t font_A[7] = {
    0x0EU, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U, 0x11U
};
static const uint8_t font_E[7] = {
    0x1FU, 0x10U, 0x10U, 0x1EU, 0x10U, 0x10U, 0x1FU
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

static const uint8_t *lcd_touch_demo_get_glyph(char c)
{
    switch (c) {
    case 'A':
        return font_A;
    case 'E':
        return font_E;
    case 'L':
        return font_L;
    case 'M':
        return font_M;
    case 'N':
        return font_N;
    case 'O':
        return font_O;
    case 'R':
        return font_R;
    case 'S':
        return font_S;
    case 'T':
        return font_T;
    case 'U':
        return font_U;
    case ' ':
    default:
        return font_space;
    }
}

static HAL_StatusTypeDef lcd_touch_demo_fill(uint8_t red,
                                             uint8_t green,
                                             uint8_t blue)
{
    return LCD_Minimal_FillColor(red, green, blue);
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

static HAL_StatusTypeDef lcd_touch_demo_draw_button(const MenuButton *button,
                                                    uint8_t active)
{
    uint8_t fill_r = button->red;
    uint8_t fill_g = button->green;
    uint8_t fill_b = button->blue;
    uint8_t border_r = UI_TITLE_R;
    uint8_t border_g = UI_TITLE_G;
    uint8_t border_b = UI_TITLE_B;
    uint16_t label_y;

    if (active != 0U) {
        fill_r = UI_BUTTON_ACTIVE_R;
        fill_g = UI_BUTTON_ACTIVE_G;
        fill_b = UI_BUTTON_ACTIVE_B;
    }

    if (LCD_Minimal_FillRect(button->x,
                             button->y,
                             button->width,
                             button->height,
                             border_r,
                             border_g,
                             border_b) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_Minimal_FillRect((uint16_t)(button->x + 6U),
                             (uint16_t)(button->y + 6U),
                             (uint16_t)(button->width - 12U),
                             (uint16_t)(button->height - 12U),
                             fill_r,
                             fill_g,
                             fill_b) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_Minimal_FillRect((uint16_t)(button->x + 16U),
                             (uint16_t)(button->y + 14U),
                             (uint16_t)(button->width - 32U),
                             10U,
                             UI_TEXT_LIGHT_R,
                             UI_TEXT_LIGHT_G,
                             UI_TEXT_LIGHT_B) != HAL_OK) {
        return HAL_ERROR;
    }

    label_y = (uint16_t)(button->y + (button->height / 2U) - ((7U * UI_BUTTON_TEXT_SCALE) / 2U));

    return lcd_touch_demo_draw_text_centered((uint16_t)(button->x + (button->width / 2U)),
                                             label_y,
                                             button->label,
                                             UI_BUTTON_TEXT_SCALE,
                                             UI_TEXT_LIGHT_R,
                                             UI_TEXT_LIGHT_G,
                                             UI_TEXT_LIGHT_B);
}

static uint8_t lcd_touch_demo_hit_test(const MenuButton *button,
                                       const Touch_XPT2046_State *touch_state)
{
    if (touch_state->valid == 0U) {
        return 0U;
    }

    if (touch_state->x < button->x || touch_state->x >= (uint16_t)(button->x + button->width)) {
        return 0U;
    }

    if (touch_state->y < button->y || touch_state->y >= (uint16_t)(button->y + button->height)) {
        return 0U;
    }

    return 1U;
}

static HAL_StatusTypeDef lcd_touch_demo_draw_menu(uint8_t active_button)
{
    MenuButton buttons[2];
    uint16_t panel_x;
    uint16_t panel_y;
    uint16_t panel_width;
    uint16_t panel_height;
    uint16_t title_y;

    panel_width = 360U;
    panel_height = 250U;
    panel_x = (uint16_t)((LCD_MINIMAL_WIDTH > panel_width) ? ((LCD_MINIMAL_WIDTH - panel_width) / 2U) : 0U);
    panel_y = 35U;

    buttons[0].x = (uint16_t)((LCD_MINIMAL_WIDTH - UI_BUTTON_WIDTH) / 2U);
    buttons[0].y = 135U;
    buttons[0].width = UI_BUTTON_WIDTH;
    buttons[0].height = UI_BUTTON_HEIGHT;
    buttons[0].label = "START";
    buttons[0].red = UI_BUTTON1_R;
    buttons[0].green = UI_BUTTON1_G;
    buttons[0].blue = UI_BUTTON1_B;

    buttons[1].x = buttons[0].x;
    buttons[1].y = (uint16_t)(buttons[0].y + UI_BUTTON_HEIGHT + UI_BUTTON_GAP);
    buttons[1].width = UI_BUTTON_WIDTH;
    buttons[1].height = UI_BUTTON_HEIGHT;
    buttons[1].label = "TOOLS";
    buttons[1].red = UI_BUTTON2_R;
    buttons[1].green = UI_BUTTON2_G;
    buttons[1].blue = UI_BUTTON2_B;

    if (lcd_touch_demo_fill(UI_BG_R, UI_BG_G, UI_BG_B) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_Minimal_FillRect(22U,
                             20U,
                             (uint16_t)(LCD_MINIMAL_WIDTH - 44U),
                             (uint16_t)(LCD_MINIMAL_HEIGHT - 40U),
                             UI_ACCENT_R,
                             UI_ACCENT_G,
                             UI_ACCENT_B) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_Minimal_FillRect(panel_x,
                             panel_y,
                             panel_width,
                             panel_height,
                             UI_CARD_R,
                             UI_CARD_G,
                             UI_CARD_B) != HAL_OK) {
        return HAL_ERROR;
    }

    title_y = 58U;
    if (lcd_touch_demo_draw_text_centered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                          title_y,
                                          "MENU",
                                          UI_TITLE_SCALE,
                                          UI_TITLE_R,
                                          UI_TITLE_G,
                                          UI_TITLE_B) != HAL_OK) {
        return HAL_ERROR;
    }

    if (lcd_touch_demo_draw_button(&buttons[0], (uint8_t)(active_button == 1U)) != HAL_OK) {
        return HAL_ERROR;
    }

    if (lcd_touch_demo_draw_button(&buttons[1], (uint8_t)(active_button == 2U)) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LCD_Touch_ColorDemo_Run(void)
{
    Touch_XPT2046_State touch_state;
    MenuButton buttons[2];
    uint8_t active_button = 0U;
    uint8_t last_active_button = 0xFFU;

    buttons[0].x = (uint16_t)((LCD_MINIMAL_WIDTH - UI_BUTTON_WIDTH) / 2U);
    buttons[0].y = 135U;
    buttons[0].width = UI_BUTTON_WIDTH;
    buttons[0].height = UI_BUTTON_HEIGHT;
    buttons[0].label = "START";
    buttons[0].red = UI_BUTTON1_R;
    buttons[0].green = UI_BUTTON1_G;
    buttons[0].blue = UI_BUTTON1_B;

    buttons[1].x = buttons[0].x;
    buttons[1].y = (uint16_t)(buttons[0].y + UI_BUTTON_HEIGHT + UI_BUTTON_GAP);
    buttons[1].width = UI_BUTTON_WIDTH;
    buttons[1].height = UI_BUTTON_HEIGHT;
    buttons[1].label = "TOOLS";
    buttons[1].red = UI_BUTTON2_R;
    buttons[1].green = UI_BUTTON2_G;
    buttons[1].blue = UI_BUTTON2_B;

    if (LCD_Minimal_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (Touch_XPT2046_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (lcd_touch_demo_draw_menu(0U) != HAL_OK) {
        return HAL_ERROR;
    }

    printf("Touch demo ready.\r\n");

    while (1) {
        if (Touch_XPT2046_ReadState(&touch_state) != HAL_OK) {
            return HAL_ERROR;
        }

        active_button = 0U;

        if (lcd_touch_demo_hit_test(&buttons[0], &touch_state) != 0U) {
            active_button = 1U;
        } else if (lcd_touch_demo_hit_test(&buttons[1], &touch_state) != 0U) {
            active_button = 2U;
        }

        if (active_button != last_active_button) {
            if (lcd_touch_demo_draw_menu(active_button) != HAL_OK) {
                return HAL_ERROR;
            }

            if (active_button == 1U) {
                printf("START selected.\r\n");
            } else if (active_button == 2U) {
                printf("TOOLS selected.\r\n");
            }

            last_active_button = active_button;
        }

        HAL_Delay(20U);
    }
}
