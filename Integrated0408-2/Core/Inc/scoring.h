#ifndef SCORING_H_
#define SCORING_H_

#include "game_types.h"

void game_reset_serve(GameContext *ctx, PlayerSide server);
void game_score_point(GameContext *ctx, PlayerSide scorer);

#endif