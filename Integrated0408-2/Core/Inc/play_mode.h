#ifndef PLAY_MODE_H
#define PLAY_MODE_H

#include <stdint.h>

#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PLAY_MODE_EVENT_NONE = 0,
    PLAY_MODE_EVENT_MATCH_FINISHED = 1
} PlayModeEvent;

typedef struct
{
    uint8_t active;
    uint32_t frame_counter;
} PlayModeContext;

void PlayMode_Init(PlayModeContext *ctx);
void PlayMode_Enter(PlayModeContext *ctx, GameContext *game);
PlayModeEvent PlayMode_Process(PlayModeContext *ctx, GameContext *game);

#ifdef __cplusplus
}
#endif

#endif /* PLAY_MODE_H */
