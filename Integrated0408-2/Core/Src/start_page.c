#include "start_page.h"

#include <string.h>

void StartPage_Init(StartPageContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->sparkle_enabled = 1U;
}

void StartPage_Enter(StartPageContext *ctx)
{
    StartPage_Init(ctx);
}

StartPageEvent StartPage_Process(StartPageContext *ctx)
{
    if (ctx == NULL) {
        return START_PAGE_EVENT_NONE;
    }

    /*
     * Skeleton only:
     * - render "tap to start"
     * - split touch area for player 1 / player 2
     * - animate sparkle
     * Those pieces can be wired in later without changing the stage contract.
     */
    if (StartPage_AreBothPlayersReady(ctx) != 0U) {
        return START_PAGE_EVENT_BOTH_READY;
    }

    return START_PAGE_EVENT_NONE;
}

void StartPage_HandleTap(StartPageContext *ctx, PlayerSide side)
{
    StartPage_SetPlayerReady(ctx, side, 1U);
}

void StartPage_SetPlayerReady(StartPageContext *ctx, PlayerSide side, uint8_t ready)
{
    if (ctx == NULL) {
        return;
    }

    if (side == SIDE_LEFT) {
        ctx->p1_ready = (ready != 0U) ? 1U : 0U;
    } else {
        ctx->p2_ready = (ready != 0U) ? 1U : 0U;
    }
}

uint8_t StartPage_IsPlayerReady(const StartPageContext *ctx, PlayerSide side)
{
    if (ctx == NULL) {
        return 0U;
    }

    return (side == SIDE_LEFT) ? ctx->p1_ready : ctx->p2_ready;
}

uint8_t StartPage_AreBothPlayersReady(const StartPageContext *ctx)
{
    if (ctx == NULL) {
        return 0U;
    }

    return (uint8_t)((ctx->p1_ready != 0U && ctx->p2_ready != 0U) ? 1U : 0U);
}

void StartPage_SetSparkleEnabled(StartPageContext *ctx, uint8_t enabled)
{
    if (ctx == NULL) {
        return;
    }

    ctx->sparkle_enabled = (enabled != 0U) ? 1U : 0U;
}
