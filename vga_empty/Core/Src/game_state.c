#include "game_state.h"

#include "game_collision.h"
#include "game_palette.h"

static void GameState_InitBody(GameBody *body, int16_t x, int16_t y, int16_t w, int16_t h, GameColor color)
{
  body->position.x = x;
  body->position.y = y;
  body->velocity.x = 0;
  body->velocity.y = 0;
  body->width = w;
  body->height = h;
  body->color = color;
}

void GameState_Init(GameState *state)
{
  if (state == 0)
  {
    return;
  }

  GameMap_LoadDefaultCourt(&state->map);
  state->score_left = 0U;
  state->score_right = 0U;
  state->running = 1U;
  GameState_ResetRally(state);
}

void GameState_ResetRally(GameState *state)
{
  int16_t floor_y;
  int16_t mid_x;

  if (state == 0)
  {
    return;
  }

  floor_y = (int16_t)((state->map.height - 2U) * state->map.tile_size);
  mid_x = (int16_t)((state->map.width * state->map.tile_size) / 2);

  GameState_InitBody(&state->left_player, 32, floor_y - 24, 12, 24, GamePalette_PlayerLeft());
  GameState_InitBody(&state->right_player, (int16_t)(mid_x + 40), floor_y - 24, 12, 24, GamePalette_PlayerRight());
  GameState_InitBody(&state->shuttle, (int16_t)(mid_x - 4), floor_y - 64, 8, 8, GamePalette_Shuttle());
  state->shuttle.velocity.x = 1;
  state->shuttle.velocity.y = -1;
}

void GameState_Tick(GameState *state)
{
  GameRect shuttle_rect;
  GameRect left_rect;
  GameRect right_rect;

  if ((state == 0) || (state->running == 0U))
  {
    return;
  }

  state->left_player.velocity.y = (int16_t)(state->left_player.velocity.y + 1);
  state->right_player.velocity.y = (int16_t)(state->right_player.velocity.y + 1);
  state->shuttle.velocity.y = (int16_t)(state->shuttle.velocity.y + 1);

  if (state->left_player.velocity.y > 3)
  {
    state->left_player.velocity.y = 3;
  }
  if (state->right_player.velocity.y > 3)
  {
    state->right_player.velocity.y = 3;
  }
  if (state->shuttle.velocity.y > 4)
  {
    state->shuttle.velocity.y = 4;
  }

  GameCollision_MoveBody(&state->left_player, &state->map);
  GameCollision_MoveBody(&state->right_player, &state->map);
  GameCollision_MoveBody(&state->shuttle, &state->map);

  shuttle_rect = GameCollision_BodyBounds(&state->shuttle);
  left_rect = GameCollision_BodyBounds(&state->left_player);
  right_rect = GameCollision_BodyBounds(&state->right_player);

  if (GameCollision_AabbIntersects(shuttle_rect, left_rect) != 0U)
  {
    state->shuttle.velocity.x = 2;
    state->shuttle.velocity.y = -3;
  }
  else if (GameCollision_AabbIntersects(shuttle_rect, right_rect) != 0U)
  {
    state->shuttle.velocity.x = -2;
    state->shuttle.velocity.y = -3;
  }
}

void GameState_SetPlayerVelocity(GameState *state, uint8_t player_index, int16_t vx, int16_t vy)
{
  GameBody *player;

  if (state == 0)
  {
    return;
  }

  player = (player_index == 0U) ? &state->left_player : &state->right_player;
  player->velocity.x = vx;
  player->velocity.y = vy;
}
