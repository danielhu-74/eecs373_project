#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>
#include <stdbool.h>


/* ── Window / render ──────────────────────────────────────── */
#define SCREEN_W   800
#define SCREEN_H   500
#define TICK_MS    16        /* ~60 fps fixed timestep */

/* ── Game constants (mirrors embedded #defines) ───────────── */
#define GRAVITY_Q8         10
#define AIR_DRAG_NUM       254
#define AIR_DRAG_DEN       256
#define SWING_VX_Q8        1800
#define SWING_VY_BASE_Q8  -900
#define MIN_REBOUND_VX_Q8  768
#define MAX_SCORE          7
#define PADDLE_SPEED       3
#define NUNCHUK_ADDR 0xA4


#define HITTING_SPEED_X 1000
#define HITTING_SPEED_Y -1000

/* ── Game states ─────────────────────────────────────────── */
typedef enum {
    STATE_MENU      = 0,
    STATE_SERVE     = 1,
    STATE_PLAYING   = 2,
    STATE_POINT     = 3,
    STATE_GAME_OVER = 4
} GameStateEnum;

typedef enum { 
    SIDE_LEFT = 0, 
    SIDE_RIGHT = 1 
} PlayerSide;

typedef struct {
    float    x, y;
    float    w, h;
    uint8_t  score;
    PlayerSide side;
    bool     move_right;
    bool     move_left;
    bool     swing;           /* rising-edge flag this tick */
    bool     swing_prev;      /* previous raw state (for edge detect) */
    float    mx, my;
} Player;

/* ── Structs (mirrors embedded game_types.h) ─────────────── */
typedef struct {
    float    x, y;
    float    w, h;
    int16_t  vx_q8, vy_q8;   /* Q8 fixed-point velocity */
    int16_t  sub_x, sub_y;   /* subpixel accumulators */
    uint8_t  radius;
    bool     in_play;
} Shuttlecock;

typedef struct {
    float x, y, w, h;
} Net;

typedef struct {
    GameStateEnum state;
    PlayerSide    last_scorer;
    PlayerSide    winner;
    uint32_t      tick_count;
    uint32_t      point_timer; /* ticks since last point scored */
} GameState;

typedef struct {
    Player      p1, p2;
    Shuttlecock shuttle;
    Net         net;
    GameState   gs;
} GameContext;

typedef struct {
    uint16_t x, y;
    bool     swing;
} PlayerData;

typedef struct {
    uint16_t x, y;
} ShuttlecockData;

typedef struct {
    uint8_t header[2];
    PlayerData player1;
    PlayerData player2;
    ShuttlecockData shuttlecock;
    uint8_t checksum;
} __attribute__((packed)) SPIData;






#endif
