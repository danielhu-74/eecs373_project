#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "stm32l4xx_hal.h"

/*
 * Central pin map for Integrated0408-2.
 *
 * Migration note:
 * - "Current project" pins are taken from Integrated0408-2.
 * - "LCD / touch migration" pins are taken from Integrated0408_trueold.
 * - User-selected GPIO pool for LCD migration:
 *   PC0, PC1, PC3, PC4, PC5
 * - To avoid the confirmed PG1 conflict with MP3_BUSY, this proposal does not
 *   use TOUCH_IRQ. The touch controller keeps only a dedicated CS line.
 */

/* -------------------------------------------------------------------------- */
/* Current project: SPI1 / FPGA data path                                     */
/* SPI1: AF5 on PA4/PA5/PA6/PA7, software CS on PD14 (output push-pull)      */
/* -------------------------------------------------------------------------- */
#define PINMAP_FPGA_SPI_NSS_Pin            GPIO_PIN_4
#define PINMAP_FPGA_SPI_NSS_GPIO_Port      GPIOA
#define PINMAP_FPGA_SPI_SCK_Pin            GPIO_PIN_5
#define PINMAP_FPGA_SPI_SCK_GPIO_Port      GPIOA
#define PINMAP_FPGA_SPI_MISO_Pin           GPIO_PIN_6
#define PINMAP_FPGA_SPI_MISO_GPIO_Port     GPIOA
#define PINMAP_FPGA_SPI_MOSI_Pin           GPIO_PIN_7
#define PINMAP_FPGA_SPI_MOSI_GPIO_Port     GPIOA

/* Software chip-select used by current spi.c */
#define PINMAP_FPGA_CS_Pin                 GPIO_PIN_14
#define PINMAP_FPGA_CS_GPIO_Port           GPIOD

/* -------------------------------------------------------------------------- */
/* LCD / TFT proposed remap from Integrated0408_trueold                       */
/* SPI3 stays on PB3/PB4/PB5                                                  */
/* User-selected GPIO control pins move to PC0/PC1/PC3/PC4                    */
/* Note: in Integrated0408-2, PC0/PC1/PC3/PC4/PC5 are currently ADC pins and  */
/* will need to be reconfigured as GPIO during the actual merge.              */
/* -------------------------------------------------------------------------- */
#define PINMAP_LCD_SPI_SCK_Pin             GPIO_PIN_3
#define PINMAP_LCD_SPI_SCK_GPIO_Port       GPIOB
#define PINMAP_LCD_SPI_MISO_Pin            GPIO_PIN_4
#define PINMAP_LCD_SPI_MISO_GPIO_Port      GPIOB
#define PINMAP_LCD_SPI_MOSI_Pin            GPIO_PIN_5
#define PINMAP_LCD_SPI_MOSI_GPIO_Port      GPIOB

#define PINMAP_LCD_CS_Pin                  GPIO_PIN_0
#define PINMAP_LCD_CS_GPIO_Port            GPIOC
#define PINMAP_LCD_DC_Pin                  GPIO_PIN_1
#define PINMAP_LCD_DC_GPIO_Port            GPIOC
#define PINMAP_LCD_RST_Pin                 GPIO_PIN_3
#define PINMAP_LCD_RST_GPIO_Port           GPIOC
#define PINMAP_LCD_BL_Pin                  GPIO_PIN_4
#define PINMAP_LCD_BL_GPIO_Port            GPIOC

/* -------------------------------------------------------------------------- */
/* Touch panel proposed remap from Integrated0408_trueold                     */
/* XPT2046 shares SPI3 clock/data with LCD                                    */
/* TOUCH_CS moves to PC5                                                      */
/* TOUCH_IRQ is intentionally left unassigned in this proposal                */
/* -------------------------------------------------------------------------- */
#define PINMAP_TOUCH_SPI_SCK_Pin           PINMAP_LCD_SPI_SCK_Pin
#define PINMAP_TOUCH_SPI_SCK_GPIO_Port     PINMAP_LCD_SPI_SCK_GPIO_Port
#define PINMAP_TOUCH_SPI_MISO_Pin          PINMAP_LCD_SPI_MISO_Pin
#define PINMAP_TOUCH_SPI_MISO_GPIO_Port    PINMAP_LCD_SPI_MISO_GPIO_Port
#define PINMAP_TOUCH_SPI_MOSI_Pin          PINMAP_LCD_SPI_MOSI_Pin
#define PINMAP_TOUCH_SPI_MOSI_GPIO_Port    PINMAP_LCD_SPI_MOSI_GPIO_Port

#define PINMAP_TOUCH_CS_Pin                GPIO_PIN_5
#define PINMAP_TOUCH_CS_GPIO_Port          GPIOC
#define PINMAP_TOUCH_IRQ_ASSIGNED          0U

/* -------------------------------------------------------------------------- */
/* trueold reference mapping for LCD / touch                                  */
/* Keep for traceability while we migrate code into the new project.          */
/* -------------------------------------------------------------------------- */
#define PINMAP_TRUEOLD_LCD_CS_Pin          GPIO_PIN_10
#define PINMAP_TRUEOLD_LCD_CS_GPIO_Port    GPIOD
#define PINMAP_TRUEOLD_LCD_DC_Pin          GPIO_PIN_11
#define PINMAP_TRUEOLD_LCD_DC_GPIO_Port    GPIOD
#define PINMAP_TRUEOLD_LCD_RST_Pin         GPIO_PIN_12
#define PINMAP_TRUEOLD_LCD_RST_GPIO_Port   GPIOD
#define PINMAP_TRUEOLD_LCD_BL_Pin          GPIO_PIN_13
#define PINMAP_TRUEOLD_LCD_BL_GPIO_Port    GPIOD

#define PINMAP_TRUEOLD_TOUCH_CS_Pin        GPIO_PIN_6
#define PINMAP_TRUEOLD_TOUCH_CS_GPIO_Port  GPIOB
#define PINMAP_TRUEOLD_TOUCH_IRQ_Pin       GPIO_PIN_1
#define PINMAP_TRUEOLD_TOUCH_IRQ_GPIO_Port GPIOG

/* -------------------------------------------------------------------------- */
/* MP3 / speaker module (USART3 + BUSY interrupt)                             */
/* USART3: AF7 on PD8/PD9, BUSY: PG1 input pull-up + EXTI1                    */
/* -------------------------------------------------------------------------- */
#define PINMAP_MP3_TX_Pin                  GPIO_PIN_8
#define PINMAP_MP3_TX_GPIO_Port            GPIOD
#define PINMAP_MP3_RX_Pin                  GPIO_PIN_9
#define PINMAP_MP3_RX_GPIO_Port            GPIOD
#define PINMAP_MP3_BUSY_Pin                GPIO_PIN_1
#define PINMAP_MP3_BUSY_GPIO_Port          GPIOG

/* -------------------------------------------------------------------------- */
/* Player 1 dance pad (PS2-style serial)                                      */
/* DATA: input pull-up, CMD/ATTN/CLK: output push-pull, idle high             */
/* -------------------------------------------------------------------------- */
#define PINMAP_PAD1_DATA_Pin               GPIO_PIN_0
#define PINMAP_PAD1_DATA_GPIO_Port         GPIOA
#define PINMAP_PAD1_CMD_Pin                GPIO_PIN_1
#define PINMAP_PAD1_CMD_GPIO_Port          GPIOA
#define PINMAP_PAD1_ATTN_Pin               GPIO_PIN_2
#define PINMAP_PAD1_ATTN_GPIO_Port         GPIOA
#define PINMAP_PAD1_CLK_Pin                GPIO_PIN_3
#define PINMAP_PAD1_CLK_GPIO_Port          GPIOA

/* -------------------------------------------------------------------------- */
/* Player 2 dance pad (added in Integrated0408-2)                             */
/* DATA: input pull-up, CMD/ATTN/CLK: output push-pull, idle high             */
/* -------------------------------------------------------------------------- */
#define PINMAP_PAD2_CLK_Pin                GPIO_PIN_4
#define PINMAP_PAD2_CLK_GPIO_Port          GPIOE
#define PINMAP_PAD2_ATTN_Pin               GPIO_PIN_5
#define PINMAP_PAD2_ATTN_GPIO_Port         GPIOE
#define PINMAP_PAD2_CMD_Pin                GPIO_PIN_6
#define PINMAP_PAD2_CMD_GPIO_Port          GPIOE
#define PINMAP_PAD2_DATA_Pin               GPIO_PIN_8
#define PINMAP_PAD2_DATA_GPIO_Port         GPIOF

/* -------------------------------------------------------------------------- */
/* Nunchuk player 1 (I2C1)                                                    */
/* I2C1: AF4 open-drain with pull-up on PB8/PB9                               */
/* -------------------------------------------------------------------------- */
#define PINMAP_NUNCHUK1_SCL_Pin            GPIO_PIN_8
#define PINMAP_NUNCHUK1_SCL_GPIO_Port      GPIOB
#define PINMAP_NUNCHUK1_SDA_Pin            GPIO_PIN_9
#define PINMAP_NUNCHUK1_SDA_GPIO_Port      GPIOB

/* -------------------------------------------------------------------------- */
/* Nunchuk player 2 (I2C2)                                                    */
/* I2C2: AF4 open-drain on PF0/PF1/PF2                                        */
/* -------------------------------------------------------------------------- */
#define PINMAP_NUNCHUK2_SDA_Pin            GPIO_PIN_0
#define PINMAP_NUNCHUK2_SDA_GPIO_Port      GPIOF
#define PINMAP_NUNCHUK2_SCL_Pin            GPIO_PIN_1
#define PINMAP_NUNCHUK2_SCL_GPIO_Port      GPIOF
#define PINMAP_NUNCHUK2_SMBA_Pin           GPIO_PIN_2
#define PINMAP_NUNCHUK2_SMBA_GPIO_Port     GPIOF

/* -------------------------------------------------------------------------- */
/* Debug console (LPUART1)                                                    */
/* LPUART1: AF8 on PG7/PG8                                                    */
/* -------------------------------------------------------------------------- */
#define PINMAP_DEBUG_TX_Pin                GPIO_PIN_7
#define PINMAP_DEBUG_TX_GPIO_Port          GPIOG
#define PINMAP_DEBUG_RX_Pin                GPIO_PIN_8
#define PINMAP_DEBUG_RX_GPIO_Port          GPIOG

/* -------------------------------------------------------------------------- */
/* Board-generated audio interface pins reserved in the .ioc                  */
/* These are configured by CubeMX but are not part of the current MP3 path.   */
/* SAI1/SAI2 alternate functions are still present in MX_GPIO_Init().         */
/* -------------------------------------------------------------------------- */
#define PINMAP_SAI1_MCLK_A_Pin             GPIO_PIN_2
#define PINMAP_SAI1_MCLK_A_GPIO_Port       GPIOE
#define PINMAP_SAI1_SD_B_Pin               GPIO_PIN_3
#define PINMAP_SAI1_SD_B_GPIO_Port         GPIOE
#define PINMAP_SAI1_MCLK_B_Pin             GPIO_PIN_7
#define PINMAP_SAI1_MCLK_B_GPIO_Port       GPIOF

#define PINMAP_SAI2_FS_A_Pin               GPIO_PIN_12
#define PINMAP_SAI2_FS_A_GPIO_Port         GPIOB
#define PINMAP_SAI2_SCK_A_Pin              GPIO_PIN_13
#define PINMAP_SAI2_SCK_A_GPIO_Port        GPIOB
#define PINMAP_SAI2_SD_A_Pin               GPIO_PIN_15
#define PINMAP_SAI2_SD_A_GPIO_Port         GPIOB
#define PINMAP_SAI2_MCLK_A_Pin             GPIO_PIN_6
#define PINMAP_SAI2_MCLK_A_GPIO_Port       GPIOC

#endif /* PIN_CONFIG_H */
