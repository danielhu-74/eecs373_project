#ifndef START_PAGE_H
#define START_PAGE_H

#include <stdint.h>

#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    START_PAGE_EVENT_NONE = 0,
    START_PAGE_EVENT_BOTH_READY = 1
} StartPageEvent;

typedef struct
{
    uint8_t p1_ready;
    uint8_t p2_ready;
    uint8_t sparkle_enabled;
} StartPageContext;

void StartPage_Init(StartPageContext *ctx);
void StartPage_Enter(StartPageContext *ctx);
StartPageEvent StartPage_Process(StartPageContext *ctx);
void StartPage_HandleTap(StartPageContext *ctx, PlayerSide side);
void StartPage_SetPlayerReady(StartPageContext *ctx, PlayerSide side, uint8_t ready);
uint8_t StartPage_IsPlayerReady(const StartPageContext *ctx, PlayerSide side);
uint8_t StartPage_AreBothPlayersReady(const StartPageContext *ctx);
void StartPage_SetSparkleEnabled(StartPageContext *ctx, uint8_t enabled);

#ifdef __cplusplus
}
#endif

#endif /* START_PAGE_H */
