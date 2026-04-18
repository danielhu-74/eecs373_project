#ifndef APP_FLOW_H
#define APP_FLOW_H

#include <stdint.h>

#include "final_stage.h"
#include "play_mode.h"
#include "start_page.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_STAGE_START_PAGE = 0,
    APP_STAGE_PLAY_MODE = 1,
    APP_STAGE_FINAL_STAGE = 2
} AppStage;

typedef struct
{
    AppStage stage;
    uint8_t stage_changed;
    StartPageContext start_page;
    PlayModeContext play_mode;
    FinalStageContext final_stage;
} AppContext;

void AppFlow_Init(AppContext *app);
void AppFlow_GoToStartPage(AppContext *app);
void AppFlow_GoToPlayMode(AppContext *app);
void AppFlow_GoToFinalStage(AppContext *app, PlayerSide winner);
AppStage AppFlow_GetStage(const AppContext *app);
uint8_t AppFlow_HasStageChanged(const AppContext *app);
void AppFlow_ClearStageChanged(AppContext *app);
const char *AppFlow_GetStageName(AppStage stage);

#ifdef __cplusplus
}
#endif

#endif /* APP_FLOW_H */
