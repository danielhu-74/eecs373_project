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
HAL_StatusTypeDef LCD_Minimal_ShowTestPattern(void);

#ifdef __cplusplus
}
#endif

#endif /* LCD_MINIMAL_H */
