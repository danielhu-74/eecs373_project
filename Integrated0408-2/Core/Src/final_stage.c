#include "final_stage.h"

#include <stdio.h>
#include <string.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "touch_xpt2046.h"

#define FINAL_STAGE_POLL_MS       20U
#define FINAL_STAGE_TITLE_SCALE   5U
#define FINAL_STAGE_LINE_SCALE    4U
#define FINAL_STAGE_PROMPT_MIN_BRIGHTNESS  0x50U
#define FINAL_STAGE_PROMPT_MAX_BRIGHTNESS  0xFFU
#define FINAL_STAGE_PROMPT_PERIOD_MS       1600U

static uint8_t final_stage_get_prompt_brightness(uint32_t now)
{
    uint32_t phase = now % FINAL_STAGE_PROMPT_PERIOD_MS;
    uint32_t half_period = FINAL_STAGE_PROMPT_PERIOD_MS / 2U;
    uint32_t range = FINAL_STAGE_PROMPT_MAX_BRIGHTNESS - FINAL_STAGE_PROMPT_MIN_BRIGHTNESS;

    if (phase < half_period) {
        return (uint8_t)(FINAL_STAGE_PROMPT_MIN_BRIGHTNESS +
                         ((range * phase) / half_period));
    }

    phase -= half_period;
    return (uint8_t)(FINAL_STAGE_PROMPT_MAX_BRIGHTNESS -
                     ((range * phase) / half_period));
}

static HAL_StatusTypeDef final_stage_clear_centered_text_area(uint16_t center_x,
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

static HAL_StatusTypeDef final_stage_render_prompt(uint8_t prompt_brightness)
{
    HAL_StatusTypeDef status;

    status = final_stage_clear_centered_text_area((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                                  250U,
                                                  "TAP TO RESTART",
                                                  FINAL_STAGE_LINE_SCALE);
    if (status != HAL_OK) {
        return status;
    }

    return LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                   250U,
                                   "TAP TO RESTART",
                                   FINAL_STAGE_LINE_SCALE,
                                   prompt_brightness,
                                   prompt_brightness,
                                   prompt_brightness);
}

static HAL_StatusTypeDef final_stage_draw_prompt_in_place(uint8_t prompt_brightness)
{
    return LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                   250U,
                                   "TAP TO RESTART",
                                   FINAL_STAGE_LINE_SCALE,
                                   prompt_brightness,
                                   prompt_brightness,
                                   prompt_brightness);
}

static void final_stage_update_prompt_animation(FinalStageContext *ctx, uint32_t now)
{
    uint8_t prompt_brightness;

    if (ctx == NULL || ctx->active == 0U) {
        return;
    }

    prompt_brightness = final_stage_get_prompt_brightness(now);
    if (prompt_brightness == ctx->prompt_brightness) {
        return;
    }

    ctx->prompt_brightness = prompt_brightness;
    (void)final_stage_draw_prompt_in_place(prompt_brightness);
}

static void final_stage_seed_touch_latch(FinalStageContext *ctx)
{
    Touch_XPT2046_State touch_state;

    if (ctx == NULL) {
        return;
    }

    if (Touch_XPT2046_ReadState(&touch_state) == HAL_OK && touch_state.valid != 0U) {
        ctx->touch_latched = 1U;
    }
}

void FinalStage_Init(FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(*ctx));
}

void FinalStage_Enter(FinalStageContext *ctx, PlayerSide winner, uint8_t p1_score, uint8_t p2_score)
{
    char winner_text[16];
    char score_text[16];

    if (ctx == NULL) {
        return;
    }

    FinalStage_Init(ctx);
    ctx->active = 1U;
    ctx->winner_valid = 1U;
    ctx->winner = winner;
    ctx->p1_score = p1_score;
    ctx->p2_score = p2_score;
    ctx->prompt_brightness = final_stage_get_prompt_brightness(HAL_GetTick());

    if (LCD_UI_Clear(0x00U, 0x00U, 0x00U) != HAL_OK) {
        return;
    }

    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  40U,
                                  "GAME OVER",
                                  FINAL_STAGE_TITLE_SCALE,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);

    (void)snprintf(winner_text,
                   sizeof(winner_text),
                   (winner == SIDE_LEFT) ? "PLAYER 1 WINS" : "PLAYER 2 WINS");
    (void)snprintf(score_text,
                   sizeof(score_text),
                   "SCORE %u-%u",
                   (unsigned int)p1_score,
                   (unsigned int)p2_score);

    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  140U,
                                  winner_text,
                                  FINAL_STAGE_LINE_SCALE,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  190U,
                                  score_text,
                                  FINAL_STAGE_LINE_SCALE,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);
    (void)final_stage_render_prompt(ctx->prompt_brightness);

    final_stage_seed_touch_latch(ctx);
}

FinalStageEvent FinalStage_Process(FinalStageContext *ctx)
{
    Touch_XPT2046_State touch_state;
    uint32_t now;

    if (ctx == NULL) {
        return FINAL_STAGE_EVENT_NONE;
    }

    if (ctx->active == 0U) {
        return FINAL_STAGE_EVENT_NONE;
    }

    if (ctx->restart_requested != 0U) {
        ctx->active = 0U;
        return FINAL_STAGE_EVENT_RESTART_REQUESTED;
    }

    now = HAL_GetTick();
    if ((now - ctx->last_poll_tick) < FINAL_STAGE_POLL_MS) {
        return FINAL_STAGE_EVENT_NONE;
    }
    ctx->last_poll_tick = now;

    if (Touch_XPT2046_ReadState(&touch_state) != HAL_OK) {
        return FINAL_STAGE_EVENT_NONE;
    }

    if (touch_state.valid != 0U) {
        if (ctx->touch_latched == 0U) {
            ctx->touch_latched = 1U;
            ctx->restart_requested = 1U;
            ctx->active = 0U;
            return FINAL_STAGE_EVENT_RESTART_REQUESTED;
        }
    } else {
        ctx->touch_latched = 0U;
    }

    final_stage_update_prompt_animation(ctx, now);

    return FINAL_STAGE_EVENT_NONE;
}

void FinalStage_RequestRestart(FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->restart_requested = 1U;
}

void FinalStage_ClearRestart(FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->restart_requested = 0U;
}

uint8_t FinalStage_HasWinner(const FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return 0U;
    }

    return ctx->winner_valid;
}

PlayerSide FinalStage_GetWinner(const FinalStageContext *ctx)
{
    if (ctx == NULL || ctx->winner_valid == 0U) {
        return SIDE_LEFT;
    }

    return ctx->winner;
}
