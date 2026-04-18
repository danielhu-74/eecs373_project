#include "app_flow.h"

#include <string.h>

static void app_flow_mark_stage(AppContext *app, AppStage stage)
{
    if (app == NULL) {
        return;
    }

    if (app->stage != stage) {
        app->stage = stage;
        app->stage_changed = 1U;
    }
}

void AppFlow_Init(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(*app));

    StartPage_Init(&app->start_page);
    PlayMode_Init(&app->play_mode);
    FinalStage_Init(&app->final_stage);

    app->stage = APP_STAGE_START_PAGE;
    app->stage_changed = 1U;
    StartPage_Enter(&app->start_page);
}

void AppFlow_GoToStartPage(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    StartPage_Enter(&app->start_page);
    app_flow_mark_stage(app, APP_STAGE_START_PAGE);
}

void AppFlow_GoToPlayMode(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    PlayMode_Enter(&app->play_mode);
    app_flow_mark_stage(app, APP_STAGE_PLAY_MODE);
}

void AppFlow_GoToFinalStage(AppContext *app, PlayerSide winner)
{
    if (app == NULL) {
        return;
    }

    FinalStage_Enter(&app->final_stage, winner);
    app_flow_mark_stage(app, APP_STAGE_FINAL_STAGE);
}

AppStage AppFlow_GetStage(const AppContext *app)
{
    if (app == NULL) {
        return APP_STAGE_START_PAGE;
    }

    return app->stage;
}

uint8_t AppFlow_HasStageChanged(const AppContext *app)
{
    if (app == NULL) {
        return 0U;
    }

    return app->stage_changed;
}

void AppFlow_ClearStageChanged(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    app->stage_changed = 0U;
}

const char *AppFlow_GetStageName(AppStage stage)
{
    switch (stage) {
    case APP_STAGE_START_PAGE:
        return "start_page";
    case APP_STAGE_PLAY_MODE:
        return "play_mode";
    case APP_STAGE_FINAL_STAGE:
        return "final_stage";
    default:
        return "unknown";
    }
}
