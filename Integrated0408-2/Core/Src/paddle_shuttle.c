#include "game_types.h"
#include <math.h>
#include <stdio.h>
#include "mp3_control.h"

/* ============================================================
 * Physics update
 * ============================================================ */
void update_shuttlecock(Shuttlecock *s)
{
    s->vy_q8 += GRAVITY_Q8;
    s->vx_q8  = (int16_t)((int32_t)s->vx_q8 * AIR_DRAG_NUM / AIR_DRAG_DEN);
    s->vy_q8  = (int16_t)((int32_t)s->vy_q8 * AIR_DRAG_NUM / AIR_DRAG_DEN);

    s->sub_x += s->vx_q8;
    s->x     += (float)(s->sub_x >> 8);
    s->sub_x &= 0xFF;

    s->sub_y += s->vy_q8;
    s->y     += (float)(s->sub_y >> 8);
    s->sub_y &= 0xFF;
}

void update_paddle(Player *p, Net *n)
{
	if (p->move_right)   p->x += PADDLE_SPEED;
	if (p->move_left) p->x -= PADDLE_SPEED;
	if (p->x < 44)            p->x = 44;
	if (p->x + p->w > 586) p->x = 554;

	if (p->side == SIDE_LEFT && p->x + p->w > n->x - 32) p->x = n->x - p->w -32;
	if (p->side == SIDE_RIGHT && p->x < n->x + n->w + 32) p->x = n->x + n->w + 32;


//    if (p->side == SIDE_LEFT){
//    	printf("Left player x: %f:\r\n", p->x);
//    }
//    else{
//    	printf("Right player x: %f:\r\n", p->x);
//    }
}


/* ============================================================
 * Collision response
 * ============================================================ */

/*
 * resolve_paddle_hit()
 *
 * Mouse swing input:
 *   The mouse cursor position relative to the paddle determines
 *   the angle of the swing, giving the player directional control.
 *
 *   mx, my = mouse cursor position this tick
 *
 *   angle = atan2(paddle_centre → cursor)
 *   vx = cos(angle) * SWING_VX_Q8   (direction toward cursor)
 *   vy = sin(angle) * SWING_VX_Q8
 *
 * Passive deflect (no swing this tick):
 *   horizontal reverse + minimum speed guarantee
 */
void resolve_paddle_hit(Shuttlecock *s, Player *p, bool swung, float mx, float my)
{
    if (swung) {
//        float pcx = p->x + p->w / 2.0f;
//        float pcy = p->y + p->h / 2.0f;
//        float dx  = mx - pcx;
//        float dy  = my - pcy;
//        float len = sqrtf(dx*dx + dy*dy);
//        if (len < 1.0f) { dx = (p->side == SIDE_LEFT) ? 1.0f : -1.0f; dy = -0.5f; len = sqrtf(dx*dx+dy*dy); }
//        float nx_dir = dx / len;
//        float ny_dir = dy / len;
//
//        /* Enforce correct horizontal direction — can't hit backward */
//        if (p->side == SIDE_LEFT  && nx_dir < 0.1f) nx_dir =  0.1f;
//        if (p->side == SIDE_RIGHT && nx_dir > -0.1f) nx_dir = -0.1f;
//
//        s->vx_q8 = (int16_t)(nx_dir * SWING_VX_Q8);
//        s->vy_q8 = (int16_t)(ny_dir * SWING_VX_Q8);
    		int16_t dir = (p->side == SIDE_LEFT) ? 1 : -1;
    	    s->vx_q8   = dir * mx;
    	    s->vy_q8   = my;
    	    Play_SFX(1, 5);
    }
//    } else {
//        /* Passive deflect */
//        s->vx_q8 = -s->vx_q8;
//        if (s->vx_q8 < MIN_REBOUND_VX_Q8 && s->vx_q8 > -MIN_REBOUND_VX_Q8)
//            s->vx_q8 = (p->side == SIDE_LEFT) ? MIN_REBOUND_VX_Q8 : -MIN_REBOUND_VX_Q8;
//    }

    /* Push shuttle out of paddle to prevent repeated collision */
    if (p->side == SIDE_LEFT)
        s->x = p->x + p->w + s->radius + 1;
    else
        s->x = p->x - s->radius - 1;
}


/* ============================================================
 * serve_launch / handle_serve_state (encapsulated per earlier refactor)
 * ============================================================ */
void serve_launch(GameContext *ctx, PlayerSide server, Player *srv)
{
    Shuttlecock *s   = &ctx->shuttle;

    if (srv->side == SIDE_RIGHT && srv->mx > 0){
    	srv->mx = -srv->mx;
    }
    s->vx_q8   = srv->mx;
    s->vy_q8   = srv->my;


    s->in_play = true;
    ctx->gs.state = STATE_PLAYING;
    Play_SFX(1, 5);
}

void handle_serve_state(GameContext *ctx)
{
    PlayerSide server  = ctx->gs.last_scorer;
    Player    *srv     = (server == SIDE_LEFT) ? &ctx->p1 : &ctx->p2;
    if (srv->swing){
        serve_launch(ctx, server, srv);
    }

}


