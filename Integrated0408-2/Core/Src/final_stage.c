#include "final_stage.h"

#include <string.h>

void FinalStage_Init(FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(*ctx));
}

void FinalStage_Enter(FinalStageContext *ctx, PlayerSide winner)
{
    if (ctx == NULL) {
        return;
    }

    FinalStage_Init(ctx);
    ctx->active = 1U;
    ctx->winner_valid = 1U;
    ctx->winner = winner;
}

FinalStageEvent FinalStage_Process(FinalStageContext *ctx)
{
    if (ctx == NULL) {
        return FINAL_STAGE_EVENT_NONE;
    }

    if (ctx->restart_requested != 0U) {
        return FINAL_STAGE_EVENT_RESTART_REQUESTED;
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
