#ifndef COLLISION_H_
#define COLLISION_H_

#include "game_types.h"

bool collision_shuttle_paddle(const Shuttlecock *s, const Player *p);
bool collision_shuttle_net(const Shuttlecock *s, const Net *n);
bool collision_shuttle_floor  (const Shuttlecock *s);
bool collision_shuttle_ceiling(const Shuttlecock *s);
bool collision_shuttle_wall(const Shuttlecock *s, PlayerSide *out);

#endif