#include "game_types.h"

/* ============================================================
 * Scoring / reset
 * ============================================================ */
void game_reset_serve(GameContext *ctx, PlayerSide server)
{
    Shuttlecock *s = &ctx->shuttle;
    Player      *p = (server == SIDE_LEFT) ? &ctx->p1 : &ctx->p2;

    s->x       = (server == SIDE_LEFT)
                 ? p->x + p->w + s->radius
                 : p->x - s->radius;
    s->y       = p->y + p->h / 2.0f;
    s->vx_q8   = 0; s->vy_q8  = 0;
    s->sub_x   = 0; s->sub_y  = 0;
    s->in_play = false;

    ctx->gs.state       = STATE_SERVE;
    ctx->gs.last_scorer = server;
    ctx->gs.point_timer = 0;

    ctx->p1.x = 76;
    ctx->p2.x = 522;
}

void game_score_point(GameContext *ctx, PlayerSide scorer)
{
    Player *wp = (scorer == SIDE_LEFT) ? &ctx->p1 : &ctx->p2;
    wp->score++;

    ctx->gs.last_scorer       = scorer;
    ctx->shuttle.in_play      = false;
    ctx->shuttle.vx_q8        = 0;
    ctx->shuttle.vy_q8        = 0;


    if (wp->score >= MAX_SCORE) {
        ctx->gs.state  = STATE_GAME_OVER;
        ctx->gs.winner = scorer;
    } else {
        ctx->gs.state       = STATE_POINT;
        ctx->gs.point_timer = 0;
    }
}
