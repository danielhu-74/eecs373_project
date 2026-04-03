#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>

#include "game_map.h"
#include "game_types.h"

typedef struct
{
  GameBody left_player;
  GameBody right_player;
  GameBody shuttle;
  GameMap map;
  uint8_t score_left;
  uint8_t score_right;
  uint8_t running;
} GameState;

void GameState_Init(GameState *state);
void GameState_ResetRally(GameState *state);
void GameState_Tick(GameState *state);
void GameState_SetPlayerVelocity(GameState *state, uint8_t player_index, int16_t vx, int16_t vy);

#endif /* GAME_STATE_H */
