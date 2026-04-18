#ifndef FINAL_STAGE_H
#define FINAL_STAGE_H

#include <stdint.h>

#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FINAL_STAGE_EVENT_NONE = 0,
    FINAL_STAGE_EVENT_RESTART_REQUESTED = 1
} FinalStageEvent;

typedef struct
{
    uint8_t active;
    uint8_t restart_requested;
    uint8_t prompt_brightness;
    uint8_t touch_latched;
    uint8_t winner_valid;
    uint8_t p1_score;
    uint8_t p2_score;
    uint32_t last_poll_tick;
    PlayerSide winner;
} FinalStageContext;

void FinalStage_Init(FinalStageContext *ctx);
void FinalStage_Enter(FinalStageContext *ctx, PlayerSide winner, uint8_t p1_score, uint8_t p2_score);
FinalStageEvent FinalStage_Process(FinalStageContext *ctx);
void FinalStage_RequestRestart(FinalStageContext *ctx);
void FinalStage_ClearRestart(FinalStageContext *ctx);
uint8_t FinalStage_HasWinner(const FinalStageContext *ctx);
PlayerSide FinalStage_GetWinner(const FinalStageContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FINAL_STAGE_H */
