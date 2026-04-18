#include "start_page.h"

#include <string.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "touch_xpt2046.h"

#define START_PAGE_POLL_MS  20U
#define START_TEXT_SCALE    6U

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
    uint16_t text_y;

    if (ctx == NULL) {
        return;
    }

    StartPage_Init(ctx);
    ctx->active = 1U;

    if (LCD_UI_Clear(0x00U, 0x00U, 0x00U) != HAL_OK) {
        return;
    }

    text_y = (uint16_t)((LCD_MINIMAL_HEIGHT - LCD_UI_GetTextHeight(START_TEXT_SCALE)) / 2U);
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  text_y,
                                  "TAP TO START",
                                  START_TEXT_SCALE,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);

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
            ctx->touch_latched = 1U;
            ctx->active = 0U;
            return START_PAGE_EVENT_START_REQUESTED;
        }
    } else {
        ctx->touch_latched = 0U;
    }

    return START_PAGE_EVENT_NONE;
}
