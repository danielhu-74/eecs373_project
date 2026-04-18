#include "lcd.h"

#define LCD_CS_LOW()    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)
#define LCD_DC_CMD()    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_DATA()   HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET)

#define LCD_GENERIC_CHUNK_PIXELS 64U
#define LCD_ILI9488_CHUNK_PIXELS 64U

SPI_HandleTypeDef hspi3;

static HAL_StatusTypeDef lcd_tx(const uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0U) {
        return HAL_OK;
    }

    return HAL_SPI_Transmit(&hspi3, (uint8_t *)data, size, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef lcd_write_repeated_565(uint16_t color, uint32_t pixels)
{
    uint8_t buf[LCD_GENERIC_CHUNK_PIXELS * 2U];
    uint32_t i;

    for (i = 0U; i < LCD_GENERIC_CHUNK_PIXELS; ++i) {
        buf[(i * 2U)] = (uint8_t)(color >> 8);
        buf[(i * 2U) + 1U] = (uint8_t)color;
    }

    LCD_DC_DATA();
    LCD_CS_LOW();

    while (pixels > 0U) {
        uint16_t count = (pixels > LCD_GENERIC_CHUNK_PIXELS) ? LCD_GENERIC_CHUNK_PIXELS : (uint16_t)pixels;
        if (lcd_tx(buf, (uint16_t)(count * 2U)) != HAL_OK) {
            LCD_CS_HIGH();
            return HAL_ERROR;
        }
        pixels -= count;
    }

    LCD_CS_HIGH();
    return HAL_OK;
}

static HAL_StatusTypeDef lcd_write_repeated_666(uint8_t red,
                                                uint8_t green,
                                                uint8_t blue,
                                                uint32_t pixels)
{
    uint8_t buf[LCD_ILI9488_CHUNK_PIXELS * 3U];
    uint32_t i;

    for (i = 0U; i < LCD_ILI9488_CHUNK_PIXELS; ++i) {
        buf[(i * 3U)] = red;
        buf[(i * 3U) + 1U] = green;
        buf[(i * 3U) + 2U] = blue;
    }

    LCD_DC_DATA();
    LCD_CS_LOW();

    while (pixels > 0U) {
        uint16_t count = (pixels > LCD_ILI9488_CHUNK_PIXELS) ? LCD_ILI9488_CHUNK_PIXELS : (uint16_t)pixels;
        if (lcd_tx(buf, (uint16_t)(count * 3U)) != HAL_OK) {
            LCD_CS_HIGH();
            return HAL_ERROR;
        }
        pixels -= count;
    }

    LCD_CS_HIGH();
    return HAL_OK;
}

HAL_StatusTypeDef LCD_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = TFT_SCK_Pin | TFT_MISO_Pin | TFT_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TFT_CS_Pin | TFT_DC_Pin | TFT_RST_Pin | TFT_BL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = 0U;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    LCD_CS_HIGH();
    LCD_DC_DATA();
    LCD_RST_HIGH();
    HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_RESET);

    return HAL_OK;
}

HAL_StatusTypeDef MX_SPI3_LCD_Init(void)
{
    __HAL_RCC_SPI3_CLK_ENABLE();

    hspi3.Instance = TFT_SPI_INSTANCE;
    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;
    hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    return HAL_SPI_Init(&hspi3);
}

HAL_StatusTypeDef LCD_Board_Init(void)
{
    if (LCD_GPIO_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (MX_SPI3_LCD_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LCD_WriteCommand(uint8_t cmd)
{
    LCD_DC_CMD();
    LCD_CS_LOW();

    if (lcd_tx(&cmd, 1U) != HAL_OK) {
        LCD_CS_HIGH();
        return HAL_ERROR;
    }

    LCD_CS_HIGH();
    return HAL_OK;
}

HAL_StatusTypeDef LCD_WriteData(uint8_t data)
{
    LCD_DC_DATA();
    LCD_CS_LOW();

    if (lcd_tx(&data, 1U) != HAL_OK) {
        LCD_CS_HIGH();
        return HAL_ERROR;
    }

    LCD_CS_HIGH();
    return HAL_OK;
}

HAL_StatusTypeDef LCD_WriteBuffer(const uint8_t *data, uint16_t size)
{
    LCD_DC_DATA();
    LCD_CS_LOW();

    if (lcd_tx(data, size) != HAL_OK) {
        LCD_CS_HIGH();
        return HAL_ERROR;
    }

    LCD_CS_HIGH();
    return HAL_OK;
}

void LCD_Reset(void)
{
    LCD_RST_LOW();
    HAL_Delay(100U);
    LCD_RST_HIGH();
    HAL_Delay(100U);
}

void LCD_BacklightSet(GPIO_PinState state)
{
    HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, state);
}

void LCD_BacklightOn(void)
{
    LCD_BacklightSet(GPIO_PIN_SET);
}

void LCD_BacklightOff(void)
{
    LCD_BacklightSet(GPIO_PIN_RESET);
}

HAL_StatusTypeDef LCD_Test_Simple(void)
{
    LCD_Reset();

    if (LCD_WriteCommand(0x11U) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(120U);

    if (LCD_WriteCommand(0x29U) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(20U);

    LCD_BacklightOn();

    while (1) {
        if (LCD_WriteCommand(0x2CU) != HAL_OK) {
            return HAL_ERROR;
        }
        if (lcd_write_repeated_565(0xF800U, (uint32_t)LCD_SIMPLE_WIDTH * LCD_SIMPLE_HEIGHT) != HAL_OK) {
            return HAL_ERROR;
        }
        HAL_Delay(1000U);

        if (LCD_WriteCommand(0x2CU) != HAL_OK) {
            return HAL_ERROR;
        }
        if (lcd_write_repeated_565(0x001FU, (uint32_t)LCD_SIMPLE_WIDTH * LCD_SIMPLE_HEIGHT) != HAL_OK) {
            return HAL_ERROR;
        }
        HAL_Delay(1000U);
    }
}

HAL_StatusTypeDef LCD_ILI9488_InitBasic(void)
{
    LCD_Reset();

    if (LCD_WriteCommand(0x11U) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(120U);

    /* Landscape mode, normal left-to-right rendering on this panel. */
    if (LCD_WriteCommand(0x36U) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LCD_WriteData(0x28U) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_WriteCommand(0x3AU) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LCD_WriteData(0x66U) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_WriteCommand(0x29U) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(20U);

    LCD_BacklightOn();
    return HAL_OK;
}

HAL_StatusTypeDef LCD_ILI9488_SetAddressWindow(uint16_t x0,
                                               uint16_t y0,
                                               uint16_t x1,
                                               uint16_t y1)
{
    uint8_t data[4];

    data[0] = (uint8_t)(x0 >> 8);
    data[1] = (uint8_t)x0;
    data[2] = (uint8_t)(x1 >> 8);
    data[3] = (uint8_t)x1;

    if (LCD_WriteCommand(0x2AU) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LCD_WriteBuffer(data, sizeof(data)) != HAL_OK) {
        return HAL_ERROR;
    }

    data[0] = (uint8_t)(y0 >> 8);
    data[1] = (uint8_t)y0;
    data[2] = (uint8_t)(y1 >> 8);
    data[3] = (uint8_t)y1;

    if (LCD_WriteCommand(0x2BU) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LCD_WriteBuffer(data, sizeof(data)) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LCD_ILI9488_FillColor(uint8_t red,
                                        uint8_t green,
                                        uint8_t blue)
{
    if (LCD_ILI9488_SetAddressWindow(0U,
                                     0U,
                                     (uint16_t)(LCD_ILI9488_WIDTH - 1U),
                                     (uint16_t)(LCD_ILI9488_HEIGHT - 1U)) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_WriteCommand(0x2CU) != HAL_OK) {
        return HAL_ERROR;
    }

    return lcd_write_repeated_666(red,
                                  green,
                                  blue,
                                  (uint32_t)LCD_ILI9488_WIDTH * LCD_ILI9488_HEIGHT);
}

HAL_StatusTypeDef LCD_ILI9488_FillRect(uint16_t x,
                                       uint16_t y,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t red,
                                       uint8_t green,
                                       uint8_t blue)
{
    uint16_t x1;
    uint16_t y1;

    if (width == 0U || height == 0U) {
        return HAL_OK;
    }

    if (x >= LCD_ILI9488_WIDTH || y >= LCD_ILI9488_HEIGHT) {
        return HAL_OK;
    }

    x1 = (uint16_t)(x + width - 1U);
    y1 = (uint16_t)(y + height - 1U);

    if (x1 >= LCD_ILI9488_WIDTH) {
        x1 = (uint16_t)(LCD_ILI9488_WIDTH - 1U);
    }

    if (y1 >= LCD_ILI9488_HEIGHT) {
        y1 = (uint16_t)(LCD_ILI9488_HEIGHT - 1U);
    }

    if (LCD_ILI9488_SetAddressWindow(x, y, x1, y1) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_WriteCommand(0x2CU) != HAL_OK) {
        return HAL_ERROR;
    }

    return lcd_write_repeated_666(red,
                                  green,
                                  blue,
                                  (uint32_t)(x1 - x + 1U) * (uint32_t)(y1 - y + 1U));
}

HAL_StatusTypeDef LCD_Test_ILI9488(void)
{
    if (LCD_ILI9488_InitBasic() != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_ILI9488_FillRect(0U,
                             0U,
                             (uint16_t)(LCD_ILI9488_WIDTH / 2U),
                             LCD_ILI9488_HEIGHT,
                             0xFFU,
                             0x00U,
                             0x00U) != HAL_OK) {
        return HAL_ERROR;
    }

    if (LCD_ILI9488_FillRect((uint16_t)(LCD_ILI9488_WIDTH / 2U),
                             0U,
                             (uint16_t)(LCD_ILI9488_WIDTH - (LCD_ILI9488_WIDTH / 2U)),
                             LCD_ILI9488_HEIGHT,
                             0x00U,
                             0x00U,
                             0xFFU) != HAL_OK) {
        return HAL_ERROR;
    }

    while (1) {
        HAL_Delay(1000U);
    }
}
