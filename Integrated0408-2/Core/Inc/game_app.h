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
    uint8_t pre_game_page_index;
    uint32_t pre_game_page_started_tick;
} GameAppContext;

void GameApp_Init(GameAppContext *ctx);
void GameApp_Process(GameAppContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GAME_APP_H */
