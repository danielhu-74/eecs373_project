#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <stdint.h>

#include "game_types.h"

#define GAME_MAP_MAX_WIDTH 64
#define GAME_MAP_MAX_HEIGHT 32

typedef enum
{
  GAME_TILE_EMPTY = 0,
  GAME_TILE_SOLID = 1,
  GAME_TILE_NET = 2,
  GAME_TILE_SPAWN_LEFT = 3,
  GAME_TILE_SPAWN_RIGHT = 4
} GameTile;

typedef struct
{
  uint8_t width;
  uint8_t height;
  uint8_t tile_size;
  GameTile tiles[GAME_MAP_MAX_HEIGHT][GAME_MAP_MAX_WIDTH];
} GameMap;

void GameMap_Clear(GameMap *map, uint8_t width, uint8_t height, uint8_t tile_size);
void GameMap_LoadDefaultCourt(GameMap *map);
void GameMap_SetTile(GameMap *map, uint8_t tx, uint8_t ty, GameTile tile);
GameTile GameMap_GetTile(const GameMap *map, uint8_t tx, uint8_t ty);
uint8_t GameMap_IsSolidTile(GameTile tile);
uint8_t GameMap_IsSolidAtPixel(const GameMap *map, int16_t px, int16_t py);

#endif /* GAME_MAP_H */
