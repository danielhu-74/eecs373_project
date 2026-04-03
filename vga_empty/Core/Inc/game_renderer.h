#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include <stdint.h>

#include "game_state.h"
#include "game_types.h"

typedef struct
{
  int16_t width;
  int16_t height;
  void (*set_pixel)(int16_t x, int16_t y, GameColor color, void *user_ctx);
  void *user_ctx;
} GameRenderTarget;

void GameRenderer_Clear(const GameRenderTarget *target, GameColor color);
void GameRenderer_DrawRect(const GameRenderTarget *target, GameRect rect, GameColor color);
void GameRenderer_DrawMap(const GameRenderTarget *target, const GameMap *map);
void GameRenderer_DrawState(const GameRenderTarget *target, const GameState *state);

#endif /* GAME_RENDERER_H */
