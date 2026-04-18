#include "play_mode.h"

#include <stddef.h>

void PlayMode_Init(PlayModeContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->reserved = 0U;
}

void PlayMode_Enter(PlayModeContext *ctx)
{
    PlayMode_Init(ctx);
}

PlayModeEvent PlayMode_Process(PlayModeContext *ctx, GameContext *game)
{
    /*
     * Intentionally left as a no-op skeleton.
     * This is the hook point for the real play-mode loop and game wiring.
     */
    (void)ctx;
    (void)game;
    return PLAY_MODE_EVENT_NONE;
}
