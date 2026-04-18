#include "final_stage.h"

#include <stdio.h>
#include <string.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "touch_xpt2046.h"

#define FINAL_STAGE_POLL_MS       20U
#define FINAL_STAGE_TITLE_SCALE   5U
#define FINAL_STAGE_LINE_SCALE    4U

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
                   (winner == SIDE_LEFT) ? "P1 WINS" : "P2 WINS");
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
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  250U,
                                  "TAP TO RESTART",
                                  FINAL_STAGE_LINE_SCALE,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);

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
