#include "game_app.h"

#include <stddef.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "mp3_control.h"

typedef struct
{
    const char *line1;
    const char *line2;
    uint16_t line1_scale;
    uint16_t line1_y;
    uint16_t line2_scale;
    uint16_t line2_y;
    uint32_t duration_ms;
} GameAppPreGamePage;

#define GAME_APP_PRE_GAME_PAGE_COUNT  5U

static const GameAppPreGamePage g_pre_game_pages[GAME_APP_PRE_GAME_PAGE_COUNT] = {
    { "LET'S START", "THE GAME", 6U, 88U, 6U, 170U, 900U },
    { "READY", NULL, 12U, 118U, 0U, 0U, 700U },
    { "1", NULL, 24U, 76U, 0U, 0U, 500U },
    { "2", NULL, 24U, 76U, 0U, 0U, 500U },
    { "3", NULL, 24U, 76U, 0U, 0U, 500U }
};

static void game_app_reset_pre_game_sequence(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->pre_game_page_index = 0U;
    ctx->pre_game_page_started_tick = 0U;
}

static void game_app_render_pre_game_page(const GameAppPreGamePage *page)
{
    if (page == NULL) {
        return;
    }

    if (LCD_UI_Clear(0x00U, 0x00U, 0x00U) != HAL_OK) {
        return;
    }

    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  page->line1_y,
                                  page->line1,
                                  page->line1_scale,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);

    if (page->line2 != NULL) {
        (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                      page->line2_y,
                                      page->line2,
                                      page->line2_scale,
                                      0xFFU,
                                      0xFFU,
                                      0xFFU);
    }
}

static void game_app_enter_pre_game_sequence(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    game_app_reset_pre_game_sequence(ctx);
    ctx->pre_game_page_started_tick = HAL_GetTick();
    game_app_render_pre_game_page(&g_pre_game_pages[0]);
    AppFlow_GoToPreGameSequence(&ctx->flow);
}

static uint8_t game_app_process_pre_game_sequence(GameAppContext *ctx)
{
    const GameAppPreGamePage *page;
    uint32_t now;

    if (ctx == NULL || ctx->pre_game_page_index >= GAME_APP_PRE_GAME_PAGE_COUNT) {
        return 1U;
    }

    page = &g_pre_game_pages[ctx->pre_game_page_index];
    now = HAL_GetTick();
    if ((now - ctx->pre_game_page_started_tick) < page->duration_ms) {
        return 0U;
    }

    ++ctx->pre_game_page_index;
    if (ctx->pre_game_page_index >= GAME_APP_PRE_GAME_PAGE_COUNT) {
        return 1U;
    }

    ctx->pre_game_page_started_tick = now;
    game_app_render_pre_game_page(&g_pre_game_pages[ctx->pre_game_page_index]);
    return 0U;
}

void GameApp_Init(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    AppFlow_Init(&ctx->flow);
    game_app_reset_pre_game_sequence(ctx);
    StartPage_Enter(&ctx->flow.start_page);
}

void GameApp_Process(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    MP3_ProcessPlaybackState();

    switch (AppFlow_GetStage(&ctx->flow)) {
    case APP_STAGE_START_PAGE:
        if (StartPage_Process(&ctx->flow.start_page) == START_PAGE_EVENT_START_REQUESTED) {
            game_app_enter_pre_game_sequence(ctx);
        }
        break;

    case APP_STAGE_PRE_GAME_SEQUENCE:
        if (game_app_process_pre_game_sequence(ctx) != 0U) {
            PlayMode_Enter(&ctx->flow.play_mode, &ctx->game);
            AppFlow_GoToPlayMode(&ctx->flow);
        }
        break;

    case APP_STAGE_PLAY_MODE:
        if (PlayMode_Process(&ctx->flow.play_mode, &ctx->game) == PLAY_MODE_EVENT_MATCH_FINISHED) {
            FinalStage_Enter(&ctx->flow.final_stage,
                             ctx->game.gs.winner,
                             ctx->game.p1.score,
                             ctx->game.p2.score);
            AppFlow_GoToFinalStage(&ctx->flow);
        }
        break;

    case APP_STAGE_FINAL_STAGE:
        if (FinalStage_Process(&ctx->flow.final_stage) == FINAL_STAGE_EVENT_RESTART_REQUESTED) {
            game_app_reset_pre_game_sequence(ctx);
            StartPage_Enter(&ctx->flow.start_page);
            AppFlow_GoToStartPage(&ctx->flow);
        }
        break;

    default:
        break;
    }
}
