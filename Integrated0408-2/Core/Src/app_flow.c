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
}

void AppFlow_GoToStartPage(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    app_flow_mark_stage(app, APP_STAGE_START_PAGE);
}

void AppFlow_GoToPreGameSequence(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    app_flow_mark_stage(app, APP_STAGE_PRE_GAME_SEQUENCE);
}

void AppFlow_GoToPlayMode(AppContext *app)
{
    if (app == NULL) {
        return;
    }

    app_flow_mark_stage(app, APP_STAGE_PLAY_MODE);
}

void AppFlow_GoToFinalStage(AppContext *app)
{
    if (app == NULL) {
        return;
    }

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
    case APP_STAGE_PRE_GAME_SEQUENCE:
        return "pre_game_sequence";
    case APP_STAGE_PLAY_MODE:
        return "play_mode";
    case APP_STAGE_FINAL_STAGE:
        return "final_stage";
    default:
        return "unknown";
    }
}
