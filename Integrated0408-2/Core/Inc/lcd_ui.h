#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef LCD_UI_EnsureReady(void);
HAL_StatusTypeDef LCD_UI_Clear(uint8_t red, uint8_t green, uint8_t blue);
HAL_StatusTypeDef LCD_UI_DrawText(uint16_t x,
                                  uint16_t y,
                                  const char *text,
                                  uint16_t scale,
                                  uint8_t red,
                                  uint8_t green,
                                  uint8_t blue);
HAL_StatusTypeDef LCD_UI_DrawTextCentered(uint16_t center_x,
                                          uint16_t y,
                                          const char *text,
                                          uint16_t scale,
                                          uint8_t red,
                                          uint8_t green,
                                          uint8_t blue);
uint16_t LCD_UI_GetTextWidth(const char *text, uint16_t scale);
uint16_t LCD_UI_GetTextHeight(uint16_t scale);

#ifdef __cplusplus
}
#endif

#endif /* LCD_UI_H */
