#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <stdint.h>

#include "game_map.h"
#include "game_types.h"

GameRect GameCollision_BodyBounds(const GameBody *body);
uint8_t GameCollision_AabbIntersects(GameRect a, GameRect b);
uint8_t GameCollision_BodyHitsMap(const GameBody *body, const GameMap *map);
void GameCollision_MoveBody(GameBody *body, const GameMap *map);

#endif /* GAME_COLLISION_H */
