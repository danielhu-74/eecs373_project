#include "touch_xpt2046.h"

#define LCD_CS_HIGH()    HAL_GPIO_WritePin(PINMAP_LCD_CS_GPIO_Port, PINMAP_LCD_CS_Pin, GPIO_PIN_SET)
#define TOUCH_CS_LOW()   HAL_GPIO_WritePin(PINMAP_TOUCH_CS_GPIO_Port, PINMAP_TOUCH_CS_Pin, GPIO_PIN_RESET)
#define TOUCH_CS_HIGH()  HAL_GPIO_WritePin(PINMAP_TOUCH_CS_GPIO_Port, PINMAP_TOUCH_CS_Pin, GPIO_PIN_SET)

#define TOUCH_CMD_READ_Y  0x90U
#define TOUCH_CMD_READ_X  0xD0U
#define TOUCH_CMD_READ_Z1 0xB0U
#define TOUCH_CMD_READ_Z2 0xC0U

#define TOUCH_SAMPLE_COUNT       7U
#define TOUCH_SPI_PRESCALER      SPI_BAUDRATEPRESCALER_64
#define TOUCH_MIN_Z1             50U
#define TOUCH_MAX_Z2             4000U
#define TOUCH_MIN_VALID_SAMPLES  2U

static uint8_t touch_last_pressed = 0U;

static void touch_deselect_all(void)
{
    LCD_CS_HIGH();
    TOUCH_CS_HIGH();
}

static uint8_t touch_pressure_is_valid(uint16_t z1, uint16_t z2)
{
    if (z1 < TOUCH_MIN_Z1 || z1 >= 4095U) {
        return 0U;
    }

    if (z2 == 0U || z2 >= TOUCH_MAX_Z2) {
        return 0U;
    }

    if (z2 <= z1) {
        return 0U;
    }

    return 1U;
}

static HAL_StatusTypeDef touch_read_value(uint8_t command, uint16_t *value)
{
    uint8_t tx[3] = {command, 0x00U, 0x00U};
    uint8_t rx[3] = {0x00U, 0x00U, 0x00U};

    if (value == NULL) {
        return HAL_ERROR;
    }

    if (HAL_SPI_TransmitReceive(&hspi3, tx, rx, 3U, HAL_MAX_DELAY) != HAL_OK) {
        return HAL_ERROR;
    }

    *value = (uint16_t)((((uint16_t)rx[1] << 8U) | rx[2]) >> 3U) & 0x0FFFU;
    return HAL_OK;
}

static HAL_StatusTypeDef touch_spi_set_prescaler(uint32_t prescaler)
{
    if (hspi3.Init.BaudRatePrescaler == prescaler) {
        return HAL_OK;
    }

    if (HAL_SPI_DeInit(&hspi3) != HAL_OK) {
        return HAL_ERROR;
    }

    hspi3.Init.BaudRatePrescaler = prescaler;

    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef touch_read_raw_once(uint16_t *raw_x,
                                             uint16_t *raw_y,
                                             uint16_t *z1,
                                             uint16_t *z2)
{
    if (raw_x == NULL || raw_y == NULL || z1 == NULL || z2 == NULL) {
        return HAL_ERROR;
    }

    touch_deselect_all();
    TOUCH_CS_LOW();

    if (touch_read_value(TOUCH_CMD_READ_Y, raw_y) != HAL_OK ||
        touch_read_value(TOUCH_CMD_READ_X, raw_x) != HAL_OK ||
        touch_read_value(TOUCH_CMD_READ_Z1, z1) != HAL_OK ||
        touch_read_value(TOUCH_CMD_READ_Z2, z2) != HAL_OK) {
        touch_deselect_all();
        return HAL_ERROR;
    }

    touch_deselect_all();
    return HAL_OK;
}

static uint16_t touch_map_axis(uint16_t raw_value,
                               uint16_t raw_min,
                               uint16_t raw_max,
                               uint16_t screen_max)
{
    uint32_t scaled;

    if (raw_value <= raw_min) {
        return 0U;
    }

    if (raw_value >= raw_max) {
        return screen_max;
    }

    scaled = ((uint32_t)(raw_value - raw_min) * screen_max) / (uint32_t)(raw_max - raw_min);
    return (uint16_t)scaled;
}

HAL_StatusTypeDef Touch_XPT2046_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = PINMAP_TOUCH_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(PINMAP_TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

#if PINMAP_TOUCH_IRQ_ASSIGNED
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Pin = PINMAP_TOUCH_IRQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PINMAP_TOUCH_IRQ_GPIO_Port, &GPIO_InitStruct);
#endif

    touch_last_pressed = 0U;
    touch_deselect_all();
    return HAL_OK;
}

GPIO_PinState Touch_XPT2046_ReadIrq(void)
{
#if PINMAP_TOUCH_IRQ_ASSIGNED
    return HAL_GPIO_ReadPin(PINMAP_TOUCH_IRQ_GPIO_Port, PINMAP_TOUCH_IRQ_Pin);
#else
    return GPIO_PIN_SET;
#endif
}

uint8_t Touch_XPT2046_IsPressed(void)
{
#if PINMAP_TOUCH_IRQ_ASSIGNED
    return (Touch_XPT2046_ReadIrq() == GPIO_PIN_RESET) ? 1U : 0U;
#else
    return touch_last_pressed;
#endif
}

HAL_StatusTypeDef Touch_XPT2046_ReadRaw(uint16_t *raw_x,
                                        uint16_t *raw_y,
                                        uint16_t *z1,
                                        uint16_t *z2)
{
    uint32_t sum_x = 0U;
    uint32_t sum_y = 0U;
    uint16_t min_x = 0x0FFFU;
    uint16_t max_x = 0U;
    uint16_t min_y = 0x0FFFU;
    uint16_t max_y = 0U;
    uint16_t sample_x = 0U;
    uint16_t sample_y = 0U;
    uint16_t sample_z1 = 0U;
    uint16_t sample_z2 = 0U;
    uint16_t last_z1 = 0U;
    uint16_t last_z2 = 0U;
    uint32_t lcd_prescaler = hspi3.Init.BaudRatePrescaler;
    uint8_t valid_samples = 0U;
    uint8_t i;

    if (raw_x == NULL || raw_y == NULL || z1 == NULL || z2 == NULL) {
        return HAL_ERROR;
    }

#if PINMAP_TOUCH_IRQ_ASSIGNED
    if (Touch_XPT2046_IsPressed() == 0U) {
        touch_last_pressed = 0U;
        return HAL_ERROR;
    }
#endif

    if (touch_spi_set_prescaler(TOUCH_SPI_PRESCALER) != HAL_OK) {
        return HAL_ERROR;
    }

    for (i = 0U; i < TOUCH_SAMPLE_COUNT; ++i) {
        if (touch_read_raw_once(&sample_x, &sample_y, &sample_z1, &sample_z2) != HAL_OK) {
            continue;
        }

        last_z1 = sample_z1;
        last_z2 = sample_z2;

        if (sample_x == 0U || sample_x >= 4095U || sample_y == 0U || sample_y >= 4095U) {
            continue;
        }

        if (touch_pressure_is_valid(sample_z1, sample_z2) == 0U) {
            continue;
        }

        if (sample_x < min_x) {
            min_x = sample_x;
        }
        if (sample_x > max_x) {
            max_x = sample_x;
        }
        if (sample_y < min_y) {
            min_y = sample_y;
        }
        if (sample_y > max_y) {
            max_y = sample_y;
        }

        sum_x += sample_x;
        sum_y += sample_y;
        ++valid_samples;
    }

    if (touch_spi_set_prescaler(lcd_prescaler) != HAL_OK) {
        return HAL_ERROR;
    }

    if (valid_samples < TOUCH_MIN_VALID_SAMPLES) {
        touch_last_pressed = 0U;
        return HAL_ERROR;
    }

    if (valid_samples > 2U) {
        *raw_x = (uint16_t)((sum_x - min_x - max_x) / (uint32_t)(valid_samples - 2U));
        *raw_y = (uint16_t)((sum_y - min_y - max_y) / (uint32_t)(valid_samples - 2U));
    } else {
        *raw_x = (uint16_t)(sum_x / valid_samples);
        *raw_y = (uint16_t)(sum_y / valid_samples);
    }

    *z1 = last_z1;
    *z2 = last_z2;
    touch_last_pressed = 1U;

    return HAL_OK;
}

HAL_StatusTypeDef Touch_XPT2046_ReadState(Touch_XPT2046_State *state)
{
    uint16_t raw_x = 0U;
    uint16_t raw_y = 0U;
    uint16_t z1 = 0U;
    uint16_t z2 = 0U;

    if (state == NULL) {
        return HAL_ERROR;
    }

    state->raw_x = 0U;
    state->raw_y = 0U;
    state->z1 = 0U;
    state->z2 = 0U;
    state->x = 0U;
    state->y = 0U;
    state->pressed = 0U;
    state->valid = 0U;

    if (Touch_XPT2046_ReadRaw(&raw_x, &raw_y, &z1, &z2) != HAL_OK) {
        return HAL_OK;
    }

    state->raw_x = raw_x;
    state->raw_y = raw_y;
    state->z1 = z1;
    state->z2 = z2;
    state->x = touch_map_axis(raw_y,
                              TOUCH_RAW_Y_MIN,
                              TOUCH_RAW_Y_MAX,
                              (uint16_t)(LCD_MINIMAL_WIDTH - 1U));
    state->y = touch_map_axis(raw_x,
                              TOUCH_RAW_X_MIN,
                              TOUCH_RAW_X_MAX,
                              (uint16_t)(LCD_MINIMAL_HEIGHT - 1U));
    state->pressed = 1U;
    state->valid = 1U;

    return HAL_OK;
}
