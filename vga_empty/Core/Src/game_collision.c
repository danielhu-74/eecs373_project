#include "game_collision.h"

GameRect GameCollision_BodyBounds(const GameBody *body)
{
  GameRect rect = {0, 0, 0, 0};

  if (body != 0)
  {
    rect.x = body->position.x;
    rect.y = body->position.y;
    rect.w = body->width;
    rect.h = body->height;
  }

  return rect;
}

uint8_t GameCollision_AabbIntersects(GameRect a, GameRect b)
{
  if ((a.x + a.w) <= b.x)
  {
    return 0U;
  }
  if ((b.x + b.w) <= a.x)
  {
    return 0U;
  }
  if ((a.y + a.h) <= b.y)
  {
    return 0U;
  }
  if ((b.y + b.h) <= a.y)
  {
    return 0U;
  }

  return 1U;
}

uint8_t GameCollision_BodyHitsMap(const GameBody *body, const GameMap *map)
{
  int16_t left;
  int16_t right;
  int16_t top;
  int16_t bottom;

  if ((body == 0) || (map == 0))
  {
    return 0U;
  }

  left = body->position.x;
  right = (int16_t)(body->position.x + body->width - 1);
  top = body->position.y;
  bottom = (int16_t)(body->position.y + body->height - 1);

  return (uint8_t)(
    GameMap_IsSolidAtPixel(map, left, top) ||
    GameMap_IsSolidAtPixel(map, right, top) ||
    GameMap_IsSolidAtPixel(map, left, bottom) ||
    GameMap_IsSolidAtPixel(map, right, bottom));
}

void GameCollision_MoveBody(GameBody *body, const GameMap *map)
{
  int16_t original_x;
  int16_t original_y;

  if ((body == 0) || (map == 0))
  {
    return;
  }

  original_x = body->position.x;
  original_y = body->position.y;

  body->position.x = (int16_t)(body->position.x + body->velocity.x);
  if (GameCollision_BodyHitsMap(body, map) != 0U)
  {
    body->position.x = original_x;
    body->velocity.x = 0;
  }

  body->position.y = (int16_t)(body->position.y + body->velocity.y);
  if (GameCollision_BodyHitsMap(body, map) != 0U)
  {
    body->position.y = original_y;
    body->velocity.y = 0;
  }
}
