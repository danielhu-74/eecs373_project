#ifndef INC_TOUCH_XPT2046_H_
#define INC_TOUCH_XPT2046_H_

#include "lcd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XPT2046 touch wiring used by this driver:
 *
 * PB3 -> T_CLK  (shared with LCD SPI3_SCK)
 * PB5 -> T_DIN  (shared with LCD SPI3_MOSI)
 * PB4 -> T_DO   (shared with LCD SPI3_MISO)
 * PB6 -> T_CS
 * PG1 -> T_IRQ  (active low, optional but recommended)
 * 3V3 -> T_VCC
 * GND -> T_GND
 */

#define TOUCH_CS_GPIO_Port      GPIOB
#define TOUCH_CS_Pin            GPIO_PIN_6
#define TOUCH_IRQ_GPIO_Port     GPIOG
#define TOUCH_IRQ_Pin           GPIO_PIN_1

/*
 * Hard-coded from the 4-point calibration you captured:
 * TL=(480,547) TR=(329,3974) BR=(3406,3799) BL=(3623,354)
 *
 * Raw X grows from top to bottom.
 * Raw Y grows from left to right.
 * So the screen mapping is:
 * screen_x <- raw_y
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
