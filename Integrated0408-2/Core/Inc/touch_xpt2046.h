#ifndef INC_TOUCH_XPT2046_H_
#define INC_TOUCH_XPT2046_H_

#include "lcd_minimal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XPT2046 wiring used by this driver in Integrated0408-2:
 *
 * PB3 -> T_CLK  (shared with LCD SPI3_SCK)
 * PB5 -> T_DIN  (shared with LCD SPI3_MOSI)
 * PB4 -> T_DO   (shared with LCD SPI3_MISO)
 * PC5 -> T_CS
 * T_IRQ is optional and intentionally unassigned in the current pin plan.
 *
 * Call LCD_Minimal_Init() before Touch_XPT2046_Init() so SPI3 is already up.
 */

/*
 * Calibration carried over from Integrated0408_trueold.
 *
 * Raw X grows from top to bottom.
 * Raw Y is used for horizontal position, but the current panel orientation
 * mirrors the X axis relative to the screen.
 * So the screen mapping is:
 * screen_x <- mirrored raw_y
 * screen_y <- raw_x
 */
#define TOUCH_RAW_X_MIN         405U
#define TOUCH_RAW_X_MAX         3515U
#define TOUCH_RAW_Y_MIN         451U
#define TOUCH_RAW_Y_MAX         3887U

typedef struct
{
    uint16_t raw_x;
    uint16_t raw_y;
    uint16_t z1;
    uint16_t z2;
    uint16_t x;
    uint16_t y;
    uint8_t pressed;
    uint8_t valid;
} Touch_XPT2046_State;

HAL_StatusTypeDef Touch_XPT2046_Init(void);
GPIO_PinState Touch_XPT2046_ReadIrq(void);
uint8_t Touch_XPT2046_IsPressed(void);
HAL_StatusTypeDef Touch_XPT2046_ReadRaw(uint16_t *raw_x,
                                        uint16_t *raw_y,
                                        uint16_t *z1,
                                        uint16_t *z2);
HAL_StatusTypeDef Touch_XPT2046_ReadState(Touch_XPT2046_State *state);

#ifdef __cplusplus
}
#endif

#endif /* INC_TOUCH_XPT2046_H_ */
