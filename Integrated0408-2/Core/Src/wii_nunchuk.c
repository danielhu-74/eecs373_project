#include "wii_nunchuk.h"

#include <stdlib.h>

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern uint8_t nunchuk_data1[6];
extern uint8_t nunchuk_data2[6];

static uint16_t last_acc_z1 = 712U;
static uint16_t last_acc_z2 = 712U;
static int cooldown_timer1 = 0;
static int cooldown_timer2 = 0;

const int SWING_THRESHOLD = 60;
const int COOLDOWN_CYCLES = 10;

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
    uint16_t accel_z;
    int16_t dz;
    int16_t abs_dz;

    if (player == NULL || nunchuk_read_p1() != HAL_OK) {
        return;
    }

    accel_z = (uint16_t)((nunchuk_data1[4] << 2) | ((nunchuk_data1[5] >> 6) & 0x03U));
    dz = (int16_t)(accel_z - last_acc_z1);
    abs_dz = (int16_t)abs(dz);

    if (cooldown_timer1 > 0) {
        cooldown_timer1--;
        if (cooldown_timer1 == 0) {
            player->swing = false;
        }
    } else if (abs_dz > SWING_THRESHOLD) {
        if (dz > SWING_THRESHOLD) {
            cooldown_timer1 = COOLDOWN_CYCLES;
            player->swing = true;
            if (player->side == SIDE_LEFT) {
                player->mx = HITTING_SPEED_X;
            } else {
                player->mx = -HITTING_SPEED_X;
            }
            player->my = HITTING_SPEED_Y;
        } else if (dz < -SWING_THRESHOLD) {
            cooldown_timer1 = COOLDOWN_CYCLES;
        }
    }

    last_acc_z1 = accel_z;
}

void process_nunchuk_p2(Player *player)
{
    uint16_t accel_z;
    int16_t dz;
    int16_t abs_dz;

    if (player == NULL || nunchuk_read_p2() != HAL_OK) {
        return;
    }

    accel_z = (uint16_t)((nunchuk_data2[4] << 2) | ((nunchuk_data2[5] >> 6) & 0x03U));
    dz = (int16_t)(accel_z - last_acc_z2);
    abs_dz = (int16_t)abs(dz);

    if (cooldown_timer2 > 0) {
        cooldown_timer2--;
        if (cooldown_timer2 == 0) {
            player->swing = false;
        }
    } else if (abs_dz > SWING_THRESHOLD) {
        if (dz > SWING_THRESHOLD) {
            cooldown_timer2 = COOLDOWN_CYCLES;
            player->swing = true;
            if (player->side == SIDE_LEFT) {
                player->mx = HITTING_SPEED_X;
            } else {
                player->mx = -HITTING_SPEED_X;
            }
            player->my = HITTING_SPEED_Y;
        } else if (dz < -SWING_THRESHOLD) {
            cooldown_timer2 = COOLDOWN_CYCLES;
            player->my = -0.707f;
        }
    }

    last_acc_z2 = accel_z;
}
