#ifndef PADDLE_SHUTTLE_H_
#define PADDLE_SHUTTLE_H_

#include "game_types.h"

void update_shuttlecock(Shuttlecock *s);
void update_paddle(Player *p, Net *n);
void resolve_paddle_hit(Shuttlecock *s, Player *p, bool swung, float mx, float my);
void serve_launch(GameContext *ctx, PlayerSide server);
void handle_serve_state(GameContext *ctx);

#endif
