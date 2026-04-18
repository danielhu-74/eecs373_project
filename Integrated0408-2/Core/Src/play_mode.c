#include "play_mode.h"

#include <stddef.h>
#include <stdio.h>

#include "ddr_pad.h"
#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "mp3_control.h"
#include "wii_nunchuk.h"
#include "game_init.h"
#include "game_update.h"

#define PLAY_LABEL_SCALE  3U
#define PLAY_SCORE_SCALE  6U
#define PLAY_SCORE_BOX_X      96U
#define PLAY_SCORE_BOX_Y      120U
#define PLAY_SCORE_BOX_WIDTH  (LCD_MINIMAL_WIDTH - 192U)
#define PLAY_SCORE_BOX_HEIGHT 64U

static HAL_StatusTypeDef play_mode_draw_static_scoreboard(void)
{
    HAL_StatusTypeDef status;

    status = LCD_UI_Clear(0x00U, 0x00U, 0x00U);
    if (status != HAL_OK) {
        return status;
    }

    status = LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 4U),
                                     40U,
                                     "PLAYER 1",
                                     PLAY_LABEL_SCALE,
                                     0xFFU,
                                     0xFFU,
                                     0xFFU);
    if (status != HAL_OK) {
        return status;
    }

    status = LCD_UI_DrawTextCentered((uint16_t)((LCD_MINIMAL_WIDTH * 3U) / 4U),
                                     40U,
                                     "PLAYER 2",
                                     PLAY_LABEL_SCALE,
                                     0xFFU,
                                     0xFFU,
                                     0xFFU);
    if (status != HAL_OK) {
        return status;
    }

    return LCD_Minimal_FillRect(48U,
                                96U,
                                (uint16_t)(LCD_MINIMAL_WIDTH - 96U),
                                2U,
                                0x30U,
                                0x30U,
                                0x30U);
}

static HAL_StatusTypeDef play_mode_render_scoreboard(PlayModeContext *ctx, const GameContext *game)
{
    char previous_score_text[16];
    char score_text[16];
    HAL_StatusTypeDef status;

    if (ctx == NULL || game == NULL) {
        return HAL_ERROR;
    }

    if (ctx->scoreboard_drawn == 0U) {
        status = play_mode_draw_static_scoreboard();
        if (status != HAL_OK) {
            return status;
        }
    }

    if (ctx->scoreboard_drawn != 0U) {
        (void)snprintf(previous_score_text,
                       sizeof(previous_score_text),
                       "%u:%u",
                       (unsigned int)ctx->last_p1_score,
                       (unsigned int)ctx->last_p2_score);

        status = LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                         132U,
                                         previous_score_text,
                                         PLAY_SCORE_SCALE,
                                         0x00U,
                                         0x00U,
                                         0x00U);
        if (status != HAL_OK) {
            return status;
        }
    }

    (void)snprintf(score_text,
                   sizeof(score_text),
                   "%u:%u",
                   (unsigned int)game->p1.score,
                   (unsigned int)game->p2.score);

    status = LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                     132U,
                                     score_text,
                                     PLAY_SCORE_SCALE,
                                     0xFFU,
                                     0xFFU,
                                     0xFFU);
    if (status != HAL_OK) {
        return status;
    }

    ctx->last_p1_score = game->p1.score;
    ctx->last_p2_score = game->p2.score;
    ctx->scoreboard_drawn = 1U;

    return HAL_OK;
}

void PlayMode_Init(PlayModeContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->active = 0U;
    ctx->last_p1_score = 0U;
    ctx->last_p2_score = 0U;
    ctx->scoreboard_drawn = 0U;
    ctx->frame_counter = 0U;
}

void PlayMode_Enter(PlayModeContext *ctx, GameContext *game)
{
    if (ctx == NULL || game == NULL) {
        return;
    }

    PlayMode_Init(ctx);
    game_init(game);
    ctx->active = 1U;

    (void)play_mode_render_scoreboard(ctx, game);
    Play_BGM(2U, 1U);
}

void PlayMode_Redraw(PlayModeContext *ctx, const GameContext *game)
{
    if (ctx == NULL || game == NULL || ctx->active == 0U) {
        return;
    }

    ctx->scoreboard_drawn = 0U;
    (void)play_mode_render_scoreboard(ctx, game);
}

PlayModeEvent PlayMode_Process(PlayModeContext *ctx, GameContext *game)
{
    if (ctx == NULL || game == NULL) {
        return PLAY_MODE_EVENT_NONE;
    }

    if (ctx->active == 0U) {
        return PLAY_MODE_EVENT_NONE;
    }

    if ((ctx->frame_counter % 48U) == 0U) {
        game->p1.swing = false;
        game->p2.swing = false;
    }

    process_nunchuk_p1(&game->p1);
    process_nunchuk_p2(&game->p2);

    process_Pad(&game->p1);
    process_Pad_P2(&game->p2);

    game_update(game);
    ++ctx->frame_counter;

    if (ctx->scoreboard_drawn == 0U ||
        ctx->last_p1_score != game->p1.score ||
        ctx->last_p2_score != game->p2.score) {
        (void)play_mode_render_scoreboard(ctx, game);
    }

    if (game->gs.state == STATE_GAME_OVER) {
        ctx->active = 0U;
        return PLAY_MODE_EVENT_MATCH_FINISHED;
    }

    return PLAY_MODE_EVENT_NONE;
}
