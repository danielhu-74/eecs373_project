#include "touch_xpt2046.h"

#define TOUCH_CS_LOW()    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET)
#define TOUCH_CS_HIGH()   HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET)

#define TOUCH_CMD_READ_Y  0x90U
#define TOUCH_CMD_READ_X  0xD0U
#define TOUCH_CMD_READ_Z1 0xB0U
#define TOUCH_CMD_READ_Z2 0xC0U

#define TOUCH_SAMPLE_COUNT 7U
#define TOUCH_SPI_PRESCALER SPI_BAUDRATEPRESCALER_64

static void touch_deselect_all(void)
{
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);
    TOUCH_CS_HIGH();
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

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Pin = TOUCH_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TOUCH_IRQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TOUCH_IRQ_GPIO_Port, &GPIO_InitStruct);

    touch_deselect_all();
    return HAL_OK;
}

GPIO_PinState Touch_XPT2046_ReadIrq(void)
{
    return HAL_GPIO_ReadPin(TOUCH_IRQ_GPIO_Port, TOUCH_IRQ_Pin);
}

uint8_t Touch_XPT2046_IsPressed(void)
{
    return (Touch_XPT2046_ReadIrq() == GPIO_PIN_RESET) ? 1U : 0U;
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

    if (Touch_XPT2046_IsPressed() == 0U) {
        return HAL_ERROR;
    }

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

    if (valid_samples == 0U) {
        (void)touch_spi_set_prescaler(lcd_prescaler);
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

    if (touch_spi_set_prescaler(lcd_prescaler) != HAL_OK) {
        return HAL_ERROR;
    }

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
    state->pressed = Touch_XPT2046_IsPressed();
    state->valid = 0U;

    if (state->pressed == 0U) {
        return HAL_OK;
    }

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
                              (uint16_t)(LCD_ILI9488_WIDTH - 1U));
    state->y = touch_map_axis(raw_x,
                              TOUCH_RAW_X_MIN,
                              TOUCH_RAW_X_MAX,
                              (uint16_t)(LCD_ILI9488_HEIGHT - 1U));
    state->valid = 1U;

    return HAL_OK;
}
