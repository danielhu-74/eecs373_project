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
    START_PAGE_EVENT_START_REQUESTED = 1
} StartPageEvent;

typedef struct
{
    uint8_t active;
    uint8_t p1_ready;
    uint8_t p2_ready;
    uint8_t prompt_brightness;
    uint8_t touch_latched;
    uint32_t last_poll_tick;
} StartPageContext;

void StartPage_Init(StartPageContext *ctx);
void StartPage_Enter(StartPageContext *ctx);
StartPageEvent StartPage_Process(StartPageContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* START_PAGE_H */
