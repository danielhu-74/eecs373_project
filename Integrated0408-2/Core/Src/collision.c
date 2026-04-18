#include "game_types.h"

/* AABB-vs-circle (precise clamp algorithm) */
bool collision_shuttle_paddle(const Shuttlecock *s, const Player *p)
{
    float nx = s->x, ny = s->y;

    if (s->x < p->x) nx = p->x;
    else if (s->x > p->x+p->w) nx = p->x + p->w;

    if (s->y < p->y)          ny = p->y;
    else if (s->y > p->y+p->h) ny = p->y + p->h;

    float dx = s->x - nx, dy = s->y - ny;
    float r = (float)s->radius;

    return (dx*dx + dy*dy) <= (r*r);
}

/* AABB vs AABB (net is thin — AABB is accurate enough) */
bool collision_shuttle_net(const Shuttlecock *s, const Net *n)
{
    float r = (float)s->radius;
    return (s->x + r >= n->x) && (s->x - r <= n->x + n->w) &&
           (s->y + r >= n->y) && (s->y - r <= n->y + n->h);
}

bool collision_shuttle_floor  (const Shuttlecock *s){ 
    return s->y + s->radius >= 400;
}

bool collision_shuttle_ceiling(const Shuttlecock *s) { 
    return s->y - s->radius <= 0.0f; 
}

bool collision_shuttle_wall(const Shuttlecock *s, PlayerSide *out)
{
    if (s->x - s->radius <= 44)      { *out = SIDE_LEFT;  return true; }
    if (s->x + s->radius >= 586)  { *out = SIDE_RIGHT; return true; }
    return false;
}
