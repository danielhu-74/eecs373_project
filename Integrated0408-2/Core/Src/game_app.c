#include "game_app.h"

#include <stddef.h>

#include "lcd_minimal.h"
#include "lcd_ui.h"
#include "mp3_control.h"
#include "spi.h"
#include "wii_nunchuk.h"

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
#define GAME_APP_WII_BUTTON_POLL_MS   40U
#define GAME_APP_PAUSE_BOX_X          88U
#define GAME_APP_PAUSE_BOX_Y          56U
#define GAME_APP_PAUSE_BOX_W          (LCD_MINIMAL_WIDTH - 176U)
#define GAME_APP_PAUSE_BOX_H          208U

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

static HAL_StatusTypeDef game_app_send_fpga_frame(const GameContext *game, uint8_t display_mode)
{
    PlayerData p1_data;
    PlayerData p2_data;
    ShuttlecockData shuttle_data;

    if (game == NULL) {
        return HAL_ERROR;
    }

    p1_data.x = (uint16_t)game->p1.x;
    p1_data.y = (uint16_t)game->p1.y;
    p1_data.swing = game->p1.swing;

    p2_data.x = (uint16_t)game->p2.x;
    p2_data.y = (uint16_t)game->p2.y;
    p2_data.swing = game->p2.swing;

    shuttle_data.x = (uint16_t)game->shuttle.x;
    shuttle_data.y = (uint16_t)game->shuttle.y;

    return SPI_SendGameState(p1_data, p2_data, shuttle_data, display_mode);
}

static void game_app_update_fpga_display(const GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    if (AppFlow_GetStage(&ctx->flow) == APP_STAGE_PLAY_MODE) {
        (void)game_app_send_fpga_frame(&ctx->game,
                                       (ctx->pause_active != 0U) ? SPI_DISPLAY_MODE_PAUSE
                                                                 : SPI_DISPLAY_MODE_PLAY);
        return;
    }

    (void)game_app_send_fpga_frame(&ctx->game, SPI_DISPLAY_MODE_BLANK);
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

static void game_app_render_pause_overlay(void)
{
    if (LCD_UI_Clear(0x00U, 0x00U, 0x00U) != HAL_OK) {
        return;
    }

    (void)LCD_Minimal_FillRect(GAME_APP_PAUSE_BOX_X,
                               GAME_APP_PAUSE_BOX_Y,
                               GAME_APP_PAUSE_BOX_W,
                               GAME_APP_PAUSE_BOX_H,
                               0x18U,
                               0x18U,
                               0x18U);
    (void)LCD_Minimal_FillRect((uint16_t)(GAME_APP_PAUSE_BOX_X + 8U),
                               (uint16_t)(GAME_APP_PAUSE_BOX_Y + 8U),
                               (uint16_t)(GAME_APP_PAUSE_BOX_W - 16U),
                               (uint16_t)(GAME_APP_PAUSE_BOX_H - 16U),
                               0x00U,
                               0x00U,
                               0x00U);

    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  84U,
                                  "PAUSED",
                                  5U,
                                  0xFFU,
                                  0xFFU,
                                  0xFFU);
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  150U,
                                  "C TO RESUME",
                                  3U,
                                  0x40U,
                                  0xFFU,
                                  0x40U);
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  202U,
                                  "Z TO RESTART",
                                  3U,
                                  0xFFU,
                                  0x60U,
                                  0x60U);
    (void)LCD_UI_DrawTextCentered((uint16_t)(LCD_MINIMAL_WIDTH / 2U),
                                  238U,
                                  "C+Z ALSO RESUMES",
                                  2U,
                                  0xA0U,
                                  0xA0U,
                                  0xA0U);
}

static void game_app_restart_to_start(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->pause_active = 0U;
    game_app_reset_pre_game_sequence(ctx);
    StartPage_Enter(&ctx->flow.start_page);
    AppFlow_GoToStartPage(&ctx->flow);
}

static void game_app_enter_pause(GameAppContext *ctx)
{
    if (ctx == NULL || AppFlow_GetStage(&ctx->flow) != APP_STAGE_PLAY_MODE) {
        return;
    }

    ctx->pause_active = 1U;
    game_app_render_pause_overlay();
}

static void game_app_leave_pause(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->pause_active = 0U;
    PlayMode_Redraw(&ctx->flow.play_mode, &ctx->game);
}

static void game_app_poll_wii_buttons(GameAppContext *ctx)
{
    NunchukButtonsState p1_state = {0U, 0U};
    NunchukButtonsState p2_state = {0U, 0U};
    uint8_t c_down = 0U;
    uint8_t z_down = 0U;
    uint32_t now;

    if (ctx == NULL) {
        return;
    }

    ctx->wii_c_pressed_event = 0U;
    ctx->wii_z_pressed_event = 0U;
    ctx->wii_combo_pressed_event = 0U;

    now = HAL_GetTick();
    if ((now - ctx->last_wii_button_poll_tick) < GAME_APP_WII_BUTTON_POLL_MS) {
        return;
    }
    ctx->last_wii_button_poll_tick = now;

    if (Nunchuk_ReadButtonsP1(&p1_state) == HAL_OK) {
        c_down |= p1_state.c_pressed;
        z_down |= p1_state.z_pressed;
    }

    if (Nunchuk_ReadButtonsP2(&p2_state) == HAL_OK) {
        c_down |= p2_state.c_pressed;
        z_down |= p2_state.z_pressed;
    }

    ctx->wii_c_pressed_event = (uint8_t)((c_down != 0U && ctx->wii_c_down == 0U) ? 1U : 0U);
    ctx->wii_z_pressed_event = (uint8_t)((z_down != 0U && ctx->wii_z_down == 0U) ? 1U : 0U);
    ctx->wii_combo_pressed_event =
        (uint8_t)(((c_down != 0U) && (z_down != 0U) &&
                   !((ctx->wii_c_down != 0U) && (ctx->wii_z_down != 0U))) ? 1U : 0U);

    ctx->wii_c_down = c_down;
    ctx->wii_z_down = z_down;
}

static void game_app_enter_pre_game_sequence(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->pause_active = 0U;
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
    ctx->pause_active = 0U;
    ctx->wii_c_down = 0U;
    ctx->wii_z_down = 0U;
    ctx->wii_c_pressed_event = 0U;
    ctx->wii_z_pressed_event = 0U;
    ctx->wii_combo_pressed_event = 0U;
    ctx->last_wii_button_poll_tick = 0U;
    StartPage_Enter(&ctx->flow.start_page);
}

void GameApp_Process(GameAppContext *ctx)
{
    if (ctx == NULL) {
        return;
    }

    MP3_ProcessPlaybackState();
    game_app_poll_wii_buttons(ctx);

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
        if (ctx->pause_active != 0U) {
            if (ctx->wii_combo_pressed_event != 0U || ctx->wii_c_pressed_event != 0U) {
                game_app_leave_pause(ctx);
            } else if (ctx->wii_z_pressed_event != 0U) {
                game_app_restart_to_start(ctx);
            }
            break;
        }

        if (ctx->wii_combo_pressed_event != 0U) {
            game_app_enter_pause(ctx);
            break;
        }

        if (PlayMode_Process(&ctx->flow.play_mode, &ctx->game) == PLAY_MODE_EVENT_MATCH_FINISHED) {
            ctx->pause_active = 0U;
            FinalStage_Enter(&ctx->flow.final_stage,
                             ctx->game.gs.winner,
                             ctx->game.p1.score,
                             ctx->game.p2.score);
            AppFlow_GoToFinalStage(&ctx->flow);
        }
        break;

    case APP_STAGE_FINAL_STAGE:
        if (ctx->wii_z_pressed_event != 0U || ctx->wii_combo_pressed_event != 0U) {
            FinalStage_RequestRestart(&ctx->flow.final_stage);
        }

        if (FinalStage_Process(&ctx->flow.final_stage) == FINAL_STAGE_EVENT_RESTART_REQUESTED) {
            game_app_restart_to_start(ctx);
        }
        break;

    default:
        break;
    }

    game_app_update_fpga_display(ctx);
}
