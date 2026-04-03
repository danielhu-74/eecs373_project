#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>

typedef enum
{
  GAME_COLOR_BLACK = 0,
  GAME_COLOR_RED = 1,
  GAME_COLOR_GREEN = 2,
  GAME_COLOR_YELLOW = 3
} GameColor;

typedef struct
{
  int16_t x;
  int16_t y;
} GameVec2i;

typedef struct
{
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} GameRect;

typedef struct
{
  GameVec2i position;
  GameVec2i velocity;
  int16_t width;
  int16_t height;
  GameColor color;
} GameBody;

#endif /* GAME_TYPES_H */
