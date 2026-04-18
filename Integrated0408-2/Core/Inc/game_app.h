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
    uint8_t pause_active;
    uint8_t wii_c_down;
    uint8_t wii_z_down;
    uint8_t wii_c_pressed_event;
    uint8_t wii_z_pressed_event;
    uint8_t wii_combo_pressed_event;
    uint32_t pre_game_page_started_tick;
    uint32_t last_wii_button_poll_tick;
} GameAppContext;

void GameApp_Init(GameAppContext *ctx);
void GameApp_Process(GameAppContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GAME_APP_H */
