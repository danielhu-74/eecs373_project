#include "game_types.h"
#include "scoring.h"
#include <string.h>

/* ============================================================
 * Initialisation
 * ============================================================ */
void game_init(GameContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->p1.x = 76;   ctx->p1.y = 336;
    ctx->p1.w = 32;   ctx->p1.h = 64;
    ctx->p1.side = SIDE_LEFT;

    ctx->p2.x = 522; ctx->p2.y = 336;
    ctx->p2.w = 32;  ctx->p2.h = 64;
    ctx->p2.side = SIDE_RIGHT;

    ctx->net.x = 313;
    ctx->net.y = 300;
    ctx->net.w = 4;
    ctx->net.h = 100;

    ctx->shuttle.radius = 8;
    ctx->gs.state = STATE_SERVE;
    ctx->gs.last_scorer = SIDE_LEFT;

    game_reset_serve(ctx, SIDE_LEFT);
}
