#include "game_app.h"

#include <stddef.h>

#include "mp3_control.h"

void GameApp_Init(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    AppFlow_Init(&ctx->flow);
    StartPage_Enter(&ctx->flow.start_page);
}

void GameApp_Process(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    MP3_ProcessPlaybackState();

    switch (AppFlow_GetStage(&ctx->flow)) {
    case APP_STAGE_START_PAGE:
        if (StartPage_Process(&ctx->flow.start_page) == START_PAGE_EVENT_START_REQUESTED) {
            PlayMode_Enter(&ctx->flow.play_mode, &ctx->game);
            AppFlow_GoToPlayMode(&ctx->flow);
        }
        break;

    case APP_STAGE_PLAY_MODE:
        if (PlayMode_Process(&ctx->flow.play_mode, &ctx->game) == PLAY_MODE_EVENT_MATCH_FINISHED) {
            FinalStage_Enter(&ctx->flow.final_stage,
                             ctx->game.gs.winner,
                             ctx->game.p1.score,
                             ctx->game.p2.score);
            AppFlow_GoToFinalStage(&ctx->flow);
        }
        break;

    case APP_STAGE_FINAL_STAGE:
        if (FinalStage_Process(&ctx->flow.final_stage) == FINAL_STAGE_EVENT_RESTART_REQUESTED) {
            StartPage_Enter(&ctx->flow.start_page);
            AppFlow_GoToStartPage(&ctx->flow);
        }
        break;

    default:
        break;
    }
}
