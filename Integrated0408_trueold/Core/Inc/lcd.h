#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Recommended wiring used by this LCD driver:
 *
 * PB3  -> TFT_SCK
 * PB5  -> TFT_MOSI / SDA / DIN
 * PB4  -> TFT_MISO (optional, not used by current code)
 * PD10 -> TFT_CS
 * PD11 -> TFT_DC
 * PD12 -> TFT_RST
 * PD13 -> TFT_BL
 *
 * Power:
 * 3V3  -> TFT_VCC
 * GND  -> TFT_GND
 */

#define TFT_SPI_INSTANCE        SPI3

#define TFT_SCK_GPIO_Port       GPIOB
#define TFT_SCK_Pin             GPIO_PIN_3
#define TFT_MISO_GPIO_Port      GPIOB
#define TFT_MISO_Pin            GPIO_PIN_4
#define TFT_MOSI_GPIO_Port      GPIOB
#define TFT_MOSI_Pin            GPIO_PIN_5

#define TFT_CS_GPIO_Port        GPIOD
#define TFT_CS_Pin              GPIO_PIN_10
#define TFT_DC_GPIO_Port        GPIOD
#define TFT_DC_Pin              GPIO_PIN_11
#define TFT_RST_GPIO_Port       GPIOD
#define TFT_RST_Pin             GPIO_PIN_12
#define TFT_BL_GPIO_Port        GPIOD
#define TFT_BL_Pin              GPIO_PIN_13

#define LCD_ILI9488_WIDTH       480U
#define LCD_ILI9488_HEIGHT      320U
#define LCD_SIMPLE_WIDTH        128U
#define LCD_SIMPLE_HEIGHT       160U

extern SPI_HandleTypeDef hspi3;

HAL_StatusTypeDef MX_SPI3_LCD_Init(void);
HAL_StatusTypeDef LCD_GPIO_Init(void);
HAL_StatusTypeDef LCD_Board_Init(void);

HAL_StatusTypeDef LCD_WriteCommand(uint8_t cmd);
HAL_StatusTypeDef LCD_WriteData(uint8_t data);
HAL_StatusTypeDef LCD_WriteBuffer(const uint8_t *data, uint16_t size);
void LCD_Reset(void);
void LCD_BacklightOn(void);
void LCD_BacklightOff(void);
void LCD_BacklightSet(GPIO_PinState state);

HAL_StatusTypeDef LCD_Test_Simple(void);

HAL_StatusTypeDef LCD_ILI9488_InitBasic(void);
HAL_StatusTypeDef LCD_ILI9488_SetAddressWindow(uint16_t x0,
                                               uint16_t y0,
                                               uint16_t x1,
                                               uint16_t y1);
HAL_StatusTypeDef LCD_ILI9488_FillRect(uint16_t x,
                                       uint16_t y,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t red,
                                       uint8_t green,
                                       uint8_t blue);
HAL_StatusTypeDef LCD_ILI9488_FillColor(uint8_t red,
                                        uint8_t green,
                                        uint8_t blue);
HAL_StatusTypeDef LCD_Test_ILI9488(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_LCD_H_ */
