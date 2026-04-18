#ifndef GAME_APP_H
#define GAME_APP_H

#include "app_flow.h"
#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    AppContext flow;
    GameContext game;
} GameAppContext;

void GameApp_Init(GameAppContext *ctx);
void GameApp_Process(GameAppContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GAME_APP_H */
