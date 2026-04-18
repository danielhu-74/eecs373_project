#include "start_page.h"

#include <string.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "touch_xpt2046.h"

#define START_PAGE_POLL_MS  20U
#define START_LABEL_SCALE   3U
#define START_STATE_SCALE   4U
#define START_ACTION_SCALE  3U

#define START_LEFT_CENTER_X   (LCD_MINIMAL_WIDTH / 4U)
#define START_RIGHT_CENTER_X  ((LCD_MINIMAL_WIDTH * 3U) / 4U)
#define START_DIVIDER_X       ((LCD_MINIMAL_WIDTH / 2U) - 1U)
#define START_SIDE_WIDTH      (LCD_MINIMAL_WIDTH / 2U)

#define START_DYNAMIC_Y       100U
#define START_DYNAMIC_HEIGHT  150U
#define START_PROMPT_MIN_BRIGHTNESS  0x50U
#define START_PROMPT_MAX_BRIGHTNESS  0xFFU
#define START_PROMPT_PERIOD_MS       1600U

static uint8_t start_page_get_prompt_brightness(uint32_t now)
{
    uint32_t phase = now % START_PROMPT_PERIOD_MS;
    uint32_t half_period = START_PROMPT_PERIOD_MS / 2U;
    uint32_t range = START_PROMPT_MAX_BRIGHTNESS - START_PROMPT_MIN_BRIGHTNESS;

    if (phase < half_period) {
        return (uint8_t)(START_PROMPT_MIN_BRIGHTNESS +
                         ((range * phase) / half_period));
    }

    phase -= half_period;
    return (uint8_t)(START_PROMPT_MAX_BRIGHTNESS -
                     ((range * phase) / half_period));
}

static HAL_StatusTypeDef start_page_clear_centered_text_area(uint16_t center_x,
                                                             uint16_t y,
                                                             const char *text,
                                                             uint16_t scale)
{
    uint16_t width;
    uint16_t height;
    uint16_t start_x;
    uint16_t clear_x;
    uint16_t clear_y;
    uint16_t clear_w;
    uint16_t clear_h;

    if (text == NULL || scale == 0U) {
        return HAL_ERROR;
    }

    width = LCD_UI_GetTextWidth(text, scale);
    height = LCD_UI_GetTextHeight(scale);
    start_x = (center_x > (width / 2U)) ? (uint16_t)(center_x - (width / 2U)) : 0U;
    clear_x = (start_x > scale) ? (uint16_t)(start_x - scale) : 0U;
    clear_y = (y > scale) ? (uint16_t)(y - scale) : 0U;
    clear_w = (uint16_t)(width + (scale * 2U));
    clear_h = (uint16_t)(height + (scale * 2U));

    return LCD_Minimal_FillRect(clear_x,
                                clear_y,
                                clear_w,
                                clear_h,
                                0x00U,
                                0x00U,
                                0x00U);
}

static HAL_StatusTypeDef start_page_clear_side_region(uint16_t center_x)
{
    uint16_t region_x = (center_x < (LCD_MINIMAL_WIDTH / 2U)) ? 0U : (uint16_t)(LCD_MINIMAL_WIDTH / 2U);

    return LCD_Minimal_FillRect(region_x,
                                START_DYNAMIC_Y,
                                START_SIDE_WIDTH,
                                START_DYNAMIC_HEIGHT,
                                0x00U,
                                0x00U,
                                0x00U);
}

static HAL_StatusTypeDef start_page_draw_static_layout(void)
{
    HAL_StatusTypeDef status;

    status = LCD_UI_Clear(0x00U, 0x00U, 0x00U);
    if (status != HAL_OK) {
        return status;
    }

    status = LCD_Minimal_FillRect(START_DIVIDER_X,
                                  24U,
                                  2U,
                                  (uint16_t)(LCD_MINIMAL_HEIGHT - 48U),
                                  0x30U,
                                  0x30U,
                                  0x30U);
    if (status != HAL_OK) {
        return status;
    }

    status = LCD_UI_DrawTextCentered((uint16_t)START_LEFT_CENTER_X,
                                     42U,
                                     "PLAYER 1",
                                     START_LABEL_SCALE,
                                     0xFFU,
                                     0xFFU,
                                     0xFFU);
    if (status != HAL_OK) {
        return status;
    }

    return LCD_UI_DrawTextCentered((uint16_t)START_RIGHT_CENTER_X,
                                   42U,
                                   "PLAYER 2",
                                   START_LABEL_SCALE,
                                   0xFFU,
                                   0xFFU,
                                   0xFFU);
}

static HAL_StatusTypeDef start_page_render_side(uint16_t center_x,
                                                uint8_t ready,
                                                uint8_t prompt_brightness)
{
    HAL_StatusTypeDef status;

    status = start_page_clear_side_region(center_x);
    if (status != HAL_OK) {
        return status;
    }

    if (ready != 0U) {
        status = LCD_UI_DrawTextCentered(center_x,
                                         118U,
                                         "READY!",
                                         START_STATE_SCALE,
                                         0x40U,
                                         0xFFU,
                                         0x40U);
        if (status != HAL_OK) {
            return status;
        }

        return LCD_UI_DrawTextCentered(center_x,
                                       204U,
                                       "TAP TO QUIT",
                                       START_ACTION_SCALE,
                                       prompt_brightness,
                                       prompt_brightness,
                                       prompt_brightness);
    }

    return LCD_UI_DrawTextCentered(center_x,
                                   150U,
                                   "TAP TO START",
                                   START_ACTION_SCALE,
                                   prompt_brightness,
                                   prompt_brightness,
                                   prompt_brightness);
}

static HAL_StatusTypeDef start_page_render_prompt(uint16_t center_x,
                                                  uint8_t ready,
                                                  uint8_t prompt_brightness)
{
    const char *prompt_text = (ready != 0U) ? "TAP TO QUIT" : "TAP TO START";
    uint16_t prompt_y = (ready != 0U) ? 204U : 150U;
    HAL_StatusTypeDef status;

    status = start_page_clear_centered_text_area(center_x,
                                                 prompt_y,
                                                 prompt_text,
                                                 START_ACTION_SCALE);
    if (status != HAL_OK) {
        return status;
    }

    return LCD_UI_DrawTextCentered(center_x,
                                   prompt_y,
                                   prompt_text,
                                   START_ACTION_SCALE,
                                   prompt_brightness,
                                   prompt_brightness,
                                   prompt_brightness);
}

static HAL_StatusTypeDef start_page_draw_prompt_in_place(uint16_t center_x,
                                                         uint8_t ready,
                                                         uint8_t prompt_brightness)
{
    const char *prompt_text = (ready != 0U) ? "TAP TO QUIT" : "TAP TO START";
    uint16_t prompt_y = (ready != 0U) ? 204U : 150U;

    return LCD_UI_DrawTextCentered(center_x,
                                   prompt_y,
                                   prompt_text,
                                   START_ACTION_SCALE,
                                   prompt_brightness,
                                   prompt_brightness,
                                   prompt_brightness);
}

static HAL_StatusTypeDef start_page_render(const StartPageContext *ctx)
{
    HAL_StatusTypeDef status;

    if (ctx == NULL) {
        return HAL_ERROR;
    }

    status = start_page_draw_static_layout();
    if (status != HAL_OK) {
        return status;
    }

    status = start_page_render_side((uint16_t)START_LEFT_CENTER_X,
                                    ctx->p1_ready,
                                    ctx->prompt_brightness);
    if (status != HAL_OK) {
        return status;
    }

    return start_page_render_side((uint16_t)START_RIGHT_CENTER_X,
                                  ctx->p2_ready,
                                  ctx->prompt_brightness);
}

static void start_page_update_prompt_animation(StartPageContext *ctx, uint32_t now)
{
    uint8_t prompt_brightness;

    if (ctx == NULL || ctx->active == 0U) {
        return;
    }

    prompt_brightness = start_page_get_prompt_brightness(now);
    if (prompt_brightness == ctx->prompt_brightness) {
        return;
    }

    ctx->prompt_brightness = prompt_brightness;
    (void)start_page_draw_prompt_in_place((uint16_t)START_LEFT_CENTER_X,
                                          ctx->p1_ready,
                                          prompt_brightness);
    (void)start_page_draw_prompt_in_place((uint16_t)START_RIGHT_CENTER_X,
                                          ctx->p2_ready,
                                          prompt_brightness);
}

static void start_page_seed_touch_latch(StartPageContext *ctx)
{
    Touch_XPT2046_State touch_state;

    if (ctx == NULL) {
        return;
    }

    if (Touch_XPT2046_ReadState(&touch_state) == HAL_OK && touch_state.valid != 0U) {
        ctx->touch_latched = 1U;
    }
}

void StartPage_Init(StartPageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(*ctx));
}

void StartPage_Enter(StartPageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    StartPage_Init(ctx);
    ctx->active = 1U;
    ctx->prompt_brightness = start_page_get_prompt_brightness(HAL_GetTick());

    if (start_page_render(ctx) != HAL_OK) {
        return;
    }

    start_page_seed_touch_latch(ctx);
}

StartPageEvent StartPage_Process(StartPageContext *ctx)
{
    Touch_XPT2046_State touch_state;
    uint32_t now;

    if (ctx == NULL) {
        return START_PAGE_EVENT_NONE;
    }

    if (ctx->active == 0U) {
        return START_PAGE_EVENT_NONE;
    }

    now = HAL_GetTick();
    if ((now - ctx->last_poll_tick) < START_PAGE_POLL_MS) {
        return START_PAGE_EVENT_NONE;
    }
    ctx->last_poll_tick = now;

    if (Touch_XPT2046_ReadState(&touch_state) != HAL_OK) {
        return START_PAGE_EVENT_NONE;
    }

    if (touch_state.valid != 0U) {
        if (ctx->touch_latched == 0U) {
            uint8_t is_left_side = (touch_state.x < (LCD_MINIMAL_WIDTH / 2U)) ? 1U : 0U;

            ctx->touch_latched = 1U;

            if (is_left_side != 0U) {
                ctx->p1_ready = (ctx->p1_ready == 0U) ? 1U : 0U;
                (void)start_page_render_side((uint16_t)START_LEFT_CENTER_X,
                                             ctx->p1_ready,
                                             ctx->prompt_brightness);
            } else {
                ctx->p2_ready = (ctx->p2_ready == 0U) ? 1U : 0U;
                (void)start_page_render_side((uint16_t)START_RIGHT_CENTER_X,
                                             ctx->p2_ready,
                                             ctx->prompt_brightness);
            }

            if (ctx->p1_ready != 0U && ctx->p2_ready != 0U) {
                ctx->active = 0U;
                return START_PAGE_EVENT_START_REQUESTED;
            }
        }
    } else {
        ctx->touch_latched = 0U;
    }

    start_page_update_prompt_animation(ctx, now);

    return START_PAGE_EVENT_NONE;
}
