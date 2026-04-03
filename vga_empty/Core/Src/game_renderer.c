#include "game_renderer.h"

#include "game_collision.h"
#include "game_palette.h"

static void GameRenderer_SetPixelSafe(const GameRenderTarget *target, int16_t x, int16_t y, GameColor color)
{
  if ((target == 0) || (target->set_pixel == 0))
  {
    return;
  }

  if ((x < 0) || (y < 0) || (x >= target->width) || (y >= target->height))
  {
    return;
  }

  target->set_pixel(x, y, color, target->user_ctx);
}

void GameRenderer_Clear(const GameRenderTarget *target, GameColor color)
{
  int16_t x;
  int16_t y;

  if (target == 0)
  {
    return;
  }

  for (y = 0; y < target->height; ++y)
  {
    for (x = 0; x < target->width; ++x)
    {
      GameRenderer_SetPixelSafe(target, x, y, color);
    }
  }
}

void GameRenderer_DrawRect(const GameRenderTarget *target, GameRect rect, GameColor color)
{
  int16_t x;
  int16_t y;

  for (y = rect.y; y < (int16_t)(rect.y + rect.h); ++y)
  {
    for (x = rect.x; x < (int16_t)(rect.x + rect.w); ++x)
    {
      GameRenderer_SetPixelSafe(target, x, y, color);
    }
  }
}

void GameRenderer_DrawMap(const GameRenderTarget *target, const GameMap *map)
{
  GameRect rect;
  uint8_t tx;
  uint8_t ty;

  if ((target == 0) || (map == 0))
  {
    return;
  }

  for (ty = 0; ty < map->height; ++ty)
  {
    for (tx = 0; tx < map->width; ++tx)
    {
      GameColor color;
      GameTile tile = GameMap_GetTile(map, tx, ty);

      if (tile == GAME_TILE_EMPTY)
      {
        continue;
      }

      color = (tile == GAME_TILE_NET) ? GamePalette_Net() : GamePalette_Court();
      rect.x = (int16_t)(tx * map->tile_size);
      rect.y = (int16_t)(ty * map->tile_size);
      rect.w = map->tile_size;
      rect.h = map->tile_size;
      GameRenderer_DrawRect(target, rect, color);
    }
  }
}

void GameRenderer_DrawState(const GameRenderTarget *target, const GameState *state)
{
  if ((target == 0) || (state == 0))
  {
    return;
  }

  GameRenderer_Clear(target, GamePalette_Background());
  GameRenderer_DrawMap(target, &state->map);
  GameRenderer_DrawRect(target, GameCollision_BodyBounds(&state->left_player), state->left_player.color);
  GameRenderer_DrawRect(target, GameCollision_BodyBounds(&state->right_player), state->right_player.color);
  GameRenderer_DrawRect(target, GameCollision_BodyBounds(&state->shuttle), state->shuttle.color);
}
