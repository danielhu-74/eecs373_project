#include "game_types.h"
#include "collision.h"
#include "game_init.h"
#include "paddle_shuttle.h"
#include "scoring.h"
#include "mp3_control.h"
#include <stdlib.h>

/* ============================================================
 * Main game update — mirrors game_update()
 * ============================================================ */
void game_update(GameContext *ctx)
{
    GameState   *gs = &ctx->gs;
    Shuttlecock *s  = &ctx->shuttle;

    gs->tick_count++;

    if (gs->state == STATE_GAME_OVER) return;

    if (gs->state == STATE_POINT) {
        gs->point_timer++;
        if (gs->point_timer > 60)   /* ~1.5 s at 60 fps */
            game_reset_serve(ctx, gs->last_scorer);
        return;
    }

    update_paddle(&ctx->p1, &ctx->net);
    update_paddle(&ctx->p2, &ctx->net);

    if (gs->state == STATE_SERVE) {
        handle_serve_state(ctx);
        /* Keep ball glued to paddle during serve */
        Player *srv = (gs->last_scorer == SIDE_LEFT) ? &ctx->p1 : &ctx->p2;
        s->x = (gs->last_scorer == SIDE_LEFT)
               ? srv->x + srv->w + s->radius + 4
               : srv->x - s->radius - 4;
        s->y = srv->y + srv->h / 2.0f;
        return;
    }

    /* ── PLAYING ── */
    update_shuttlecock(s);


    /* Ceiling */
    if (collision_shuttle_ceiling(s)) {
        s->y     = (float)s->radius;
        s->vy_q8 = (int16_t)abs(s->vy_q8);
    }

    /* Net */
    if (collision_shuttle_net(s, &ctx->net)) {
        PlayerSide loser = (s->vx_q8 > 0) ? SIDE_LEFT : SIDE_RIGHT;
        game_score_point(ctx, loser == SIDE_LEFT ? SIDE_RIGHT : SIDE_LEFT);
        //Play_SFX(1, 4);
        return;
    }

    /* Paddle hits */
    if (collision_shuttle_paddle(s, &ctx->p1)){
        //printf("player1 swung? %d\n", ctx->p1.swing);
        resolve_paddle_hit(s, &ctx->p1, ctx->p1.swing, ctx->p1.mx, ctx->p2.my);
    }

    if (collision_shuttle_paddle(s, &ctx->p2)){
        //printf("player2 swung? %d\n", ctx->p2.swing);
        resolve_paddle_hit(s, &ctx->p2, ctx->p2.swing, ctx->p2.mx, ctx->p2.my);
    }


    /* Floor */
    if (collision_shuttle_floor(s)) {
        PlayerSide loser = (s->x < ctx->net.x) ? SIDE_LEFT : SIDE_RIGHT;
        game_score_point(ctx, loser == SIDE_LEFT ? SIDE_RIGHT : SIDE_LEFT);
        //Play_SFX(1, 4);
        //HAL_Delay(500);
        return;
    }

    /* Walls */
    PlayerSide wall_side;
    if (collision_shuttle_wall(s, &wall_side)) {
        game_score_point(ctx, wall_side == SIDE_LEFT ? SIDE_LEFT : SIDE_RIGHT);
        //Play_SFX(1, 4);
        return;
    }

}
