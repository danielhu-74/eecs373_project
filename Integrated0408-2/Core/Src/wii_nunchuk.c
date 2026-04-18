#include "wii_nunchuk.h"

#include <stdlib.h>

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern uint8_t nunchuk_data1[6];
extern uint8_t nunchuk_data2[6];

typedef struct
{
    uint16_t last_acc_z;
    uint16_t recent_peak_dz;
    int cooldown_timer;
} NunchukSwingContext;

static NunchukSwingContext g_p1_swing_ctx = { 712U, 0U, 0 };
static NunchukSwingContext g_p2_swing_ctx = { 712U, 0U, 0 };

const int SWING_THRESHOLD = 60;
const int COOLDOWN_CYCLES = 10;

#define SWING_PEAK_DECAY             10U
#define SWING_POWER_MAX_DZ           220U
#define SWING_POWER_MIN_PERCENT      85U
#define SWING_POWER_MAX_PERCENT      185U

static HAL_StatusTypeDef nunchuk_read_raw(I2C_HandleTypeDef *hi2c, uint8_t *buffer)
{
    uint8_t request_data = 0x00;
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(hi2c, NUNCHUK_ADDR, &request_data, 1, 100);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(2);
    return HAL_I2C_Master_Receive(hi2c, NUNCHUK_ADDR, buffer, 6, 100);
}

static HAL_StatusTypeDef nunchuk_read_p1(void)
{
    return nunchuk_read_raw(&hi2c1, nunchuk_data1);
}

static HAL_StatusTypeDef nunchuk_read_p2(void)
{
    return nunchuk_read_raw(&hi2c2, nunchuk_data2);
}

static void nunchuk_decode_buttons(const uint8_t *buffer, NunchukButtonsState *state)
{
    if (buffer == NULL || state == NULL) {
        return;
    }

    state->z_pressed = ((buffer[5] & 0x01U) == 0U) ? 1U : 0U;
    state->c_pressed = ((buffer[5] & 0x02U) == 0U) ? 1U : 0U;
}

static uint16_t nunchuk_track_recent_peak(NunchukSwingContext *ctx, uint16_t abs_dz)
{
    if (ctx == NULL) {
        return abs_dz;
    }

    if (ctx->recent_peak_dz > SWING_PEAK_DECAY) {
        ctx->recent_peak_dz = (uint16_t)(ctx->recent_peak_dz - SWING_PEAK_DECAY);
    } else {
        ctx->recent_peak_dz = 0U;
    }

    if (abs_dz > ctx->recent_peak_dz) {
        ctx->recent_peak_dz = abs_dz;
    }

    return ctx->recent_peak_dz;
}

static uint16_t nunchuk_compute_speed_percent(uint16_t peak_dz)
{
    uint32_t clamped_peak;
    uint32_t scale_percent;

    if (peak_dz <= (uint16_t)SWING_THRESHOLD) {
        return SWING_POWER_MIN_PERCENT;
    }

    clamped_peak = peak_dz;
    if (clamped_peak > SWING_POWER_MAX_DZ) {
        clamped_peak = SWING_POWER_MAX_DZ;
    }

    scale_percent = SWING_POWER_MIN_PERCENT +
        ((clamped_peak - (uint32_t)SWING_THRESHOLD) *
         (uint32_t)(SWING_POWER_MAX_PERCENT - SWING_POWER_MIN_PERCENT)) /
        (uint32_t)(SWING_POWER_MAX_DZ - SWING_THRESHOLD);

    return (uint16_t)scale_percent;
}

static void nunchuk_apply_swing_velocity(Player *player, uint16_t peak_dz)
{
    uint16_t speed_percent;

    if (player == NULL) {
        return;
    }

    speed_percent = nunchuk_compute_speed_percent(peak_dz);
    player->mx = ((float)HITTING_SPEED_X * (float)speed_percent) / 100.0f;
    player->my = ((float)HITTING_SPEED_Y * (float)speed_percent) / 100.0f;
}

static void nunchuk_process_swing(Player *player,
                                  const uint8_t *buffer,
                                  NunchukSwingContext *ctx)
{
    uint16_t accel_z;
    int16_t dz;
    uint16_t abs_dz;
    uint16_t peak_dz;

    if (player == NULL || buffer == NULL || ctx == NULL) {
        return;
    }

    accel_z = (uint16_t)((buffer[4] << 2) | ((buffer[5] >> 6) & 0x03U));
    dz = (int16_t)(accel_z - ctx->last_acc_z);
    abs_dz = (uint16_t)abs(dz);
    peak_dz = nunchuk_track_recent_peak(ctx, abs_dz);
    ctx->last_acc_z = accel_z;

    if (ctx->cooldown_timer > 0) {
        ctx->cooldown_timer--;
        if (ctx->cooldown_timer == 0) {
            player->swing = false;
        }
        return;
    }

    if (dz > SWING_THRESHOLD) {
        ctx->cooldown_timer = COOLDOWN_CYCLES;
        player->swing = true;
        nunchuk_apply_swing_velocity(player, peak_dz);
        ctx->recent_peak_dz = 0U;
    }
}

void nunchuk_init(void)
{
    uint8_t init_data1[2] = {0xF0, 0x55};
    uint8_t init_data2[2] = {0xFB, 0x00};

    HAL_I2C_Master_Transmit(&hi2c1, NUNCHUK_ADDR, init_data1, 2, 100);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c1, NUNCHUK_ADDR, init_data2, 2, 100);
    HAL_Delay(10);

    HAL_I2C_Master_Transmit(&hi2c2, NUNCHUK_ADDR, init_data1, 2, 100);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c2, NUNCHUK_ADDR, init_data2, 2, 100);
    HAL_Delay(10);
}

HAL_StatusTypeDef Nunchuk_ReadButtonsP1(NunchukButtonsState *state)
{
    HAL_StatusTypeDef status;

    if (state == NULL) {
        return HAL_ERROR;
    }

    status = nunchuk_read_p1();
    if (status != HAL_OK) {
        state->c_pressed = 0U;
        state->z_pressed = 0U;
        return status;
    }

    nunchuk_decode_buttons(nunchuk_data1, state);
    return HAL_OK;
}

HAL_StatusTypeDef Nunchuk_ReadButtonsP2(NunchukButtonsState *state)
{
    HAL_StatusTypeDef status;

    if (state == NULL) {
        return HAL_ERROR;
    }

    status = nunchuk_read_p2();
    if (status != HAL_OK) {
        state->c_pressed = 0U;
        state->z_pressed = 0U;
        return status;
    }

    nunchuk_decode_buttons(nunchuk_data2, state);
    return HAL_OK;
}

void process_nunchuk_p1(Player *player)
{
    if (player == NULL || nunchuk_read_p1() != HAL_OK) {
        return;
    }

    nunchuk_process_swing(player, nunchuk_data1, &g_p1_swing_ctx);
}

void process_nunchuk_p2(Player *player)
{
    if (player == NULL || nunchuk_read_p2() != HAL_OK) {
        return;
    }

    nunchuk_process_swing(player, nunchuk_data2, &g_p2_swing_ctx);
}
