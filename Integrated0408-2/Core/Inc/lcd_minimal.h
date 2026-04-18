#ifndef LCD_MINIMAL_H
#define LCD_MINIMAL_H

#include "main.h"
#include "pin_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_MINIMAL_WIDTH   480U
#define LCD_MINIMAL_HEIGHT  320U

extern SPI_HandleTypeDef hspi3;

HAL_StatusTypeDef LCD_Minimal_Init(void);
HAL_StatusTypeDef LCD_Minimal_FillColor(uint8_t red, uint8_t green, uint8_t blue);
HAL_StatusTypeDef LCD_Minimal_FillRect(uint16_t x,
                                       uint16_t y,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t red,
                                       uint8_t green,
                                       uint8_t blue);
HAL_StatusTypeDef LCD_Minimal_ShowTestPattern(void);
void LCD_Minimal_BacklightSet(GPIO_PinState state);
void LCD_Minimal_BacklightOn(void);
void LCD_Minimal_BacklightOff(void);

#ifdef __cplusplus
}
#endif

#endif /* LCD_MINIMAL_H */
