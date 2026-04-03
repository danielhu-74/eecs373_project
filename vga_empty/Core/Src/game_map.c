#include "game_map.h"

void GameMap_Clear(GameMap *map, uint8_t width, uint8_t height, uint8_t tile_size)
{
  uint8_t x;
  uint8_t y;

  if ((map == 0) || (width > GAME_MAP_MAX_WIDTH) || (height > GAME_MAP_MAX_HEIGHT) || (tile_size == 0U))
  {
    return;
  }

  map->width = width;
  map->height = height;
  map->tile_size = tile_size;

  for (y = 0; y < GAME_MAP_MAX_HEIGHT; ++y)
  {
    for (x = 0; x < GAME_MAP_MAX_WIDTH; ++x)
    {
      map->tiles[y][x] = GAME_TILE_EMPTY;
    }
  }
}

void GameMap_LoadDefaultCourt(GameMap *map)
{
  uint8_t x;
  uint8_t y;
  uint8_t net_x;

  if (map == 0)
  {
    return;
  }

  GameMap_Clear(map, 40U, 24U, 8U);
  net_x = (uint8_t)(map->width / 2U);

  for (x = 0; x < map->width; ++x)
  {
    map->tiles[map->height - 1U][x] = GAME_TILE_SOLID;
  }

  for (y = (uint8_t)(map->height - 6U); y < (uint8_t)(map->height - 1U); ++y)
  {
    map->tiles[y][net_x] = GAME_TILE_NET;
  }

  map->tiles[map->height - 2U][2U] = GAME_TILE_SPAWN_LEFT;
  map->tiles[map->height - 2U][(uint8_t)(map->width - 3U)] = GAME_TILE_SPAWN_RIGHT;
}

void GameMap_SetTile(GameMap *map, uint8_t tx, uint8_t ty, GameTile tile)
{
  if ((map == 0) || (tx >= map->width) || (ty >= map->height))
  {
    return;
  }

  map->tiles[ty][tx] = tile;
}

GameTile GameMap_GetTile(const GameMap *map, uint8_t tx, uint8_t ty)
{
  if ((map == 0) || (tx >= map->width) || (ty >= map->height))
  {
    return GAME_TILE_SOLID;
  }

  return map->tiles[ty][tx];
}

uint8_t GameMap_IsSolidTile(GameTile tile)
{
  return (uint8_t)((tile == GAME_TILE_SOLID) || (tile == GAME_TILE_NET));
}

uint8_t GameMap_IsSolidAtPixel(const GameMap *map, int16_t px, int16_t py)
{
  uint8_t tx;
  uint8_t ty;

  if ((map == 0) || (px < 0) || (py < 0))
  {
    return 1U;
  }

  tx = (uint8_t)(px / map->tile_size);
  ty = (uint8_t)(py / map->tile_size);
  return GameMap_IsSolidTile(GameMap_GetTile(map, tx, ty));
}
