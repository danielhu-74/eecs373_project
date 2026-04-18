#include "play_mode.h"

#include <stddef.h>

#include "ddr_pad.h"
#include "lcd_ui.h"
#include "mp3_control.h"
#include "spi.h"
#include "wii_nunchuk.h"
#include "game_init.h"
#include "game_update.h"

void PlayMode_Init(PlayModeContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->active = 0U;
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

    (void)LCD_UI_Clear(0x00U, 0x00U, 0x00U);
    Play_BGM(2U, 1U);
}

PlayModeEvent PlayMode_Process(PlayModeContext *ctx, GameContext *game)
{
    PlayerData p1_data;
    PlayerData p2_data;
    ShuttlecockData shuttle_data;

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

    p1_data.x = (uint16_t)game->p1.x;
    p1_data.y = (uint16_t)game->p1.y;
    p1_data.swing = game->p1.swing;

    p2_data.x = (uint16_t)game->p2.x;
    p2_data.y = (uint16_t)game->p2.y;
    p2_data.swing = game->p2.swing;

    shuttle_data.x = (uint16_t)game->shuttle.x;
    shuttle_data.y = (uint16_t)game->shuttle.y;

    (void)SPI_SendGameState(p1_data, p2_data, shuttle_data);

    if (game->gs.state == STATE_GAME_OVER) {
        ctx->active = 0U;
        return PLAY_MODE_EVENT_MATCH_FINISHED;
    }

    return PLAY_MODE_EVENT_NONE;
}
