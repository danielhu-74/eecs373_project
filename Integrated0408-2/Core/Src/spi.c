/*
 * spi.c
 *
 *  Created on: Apr 7, 2026
 *      Author: jhsbzd
 */

#include "game_types.h"
#include "spi.h"
#include "main.h"


#define FPGA_CS_Pin GPIO_PIN_14
#define FPGA_CS_GPIO_Port GPIOD

extern SPI_HandleTypeDef hspi1;

uint8_t tx[13];

//static uint8_t calc_checksum(const uint8_t *data, uint16_t len) {
//    uint8_t sum = 0;
//    for (uint16_t i = 0; i < len; i++) sum ^= data[i];
//    return sum;
//}

HAL_StatusTypeDef SPI_SendGameState(PlayerData p1,
                                    PlayerData p2,
                                    ShuttlecockData sc,
                                    uint8_t display_mode)
{
    uint8_t state_byte;
    HAL_StatusTypeDef spi_status;

    tx[0]  = (p1.x >> 8) & 0xFF;
	tx[1]  = p1.x & 0xFF;
	tx[2]  = (p1.y >> 8) & 0xFF;
	tx[3]  = p1.y & 0xFF;

	tx[4]  = (p2.x >> 8) & 0xFF;
	tx[5]  = p2.x & 0xFF;
	tx[6]  = (p2.y >> 8) & 0xFF;
	tx[7]  = p2.y & 0xFF;

	tx[8]  = (sc.x >> 8) & 0xFF;
	tx[9]  = sc.x & 0xFF;
	tx[10] = (sc.y >> 8) & 0xFF;
	tx[11] = sc.y & 0xFF;
    state_byte = (uint8_t)(((display_mode & 0x07U) << 2U) |
                           ((p2.swing != false) ? 0x02U : 0x00U) |
                           ((p1.swing != false) ? 0x01U : 0x00U));
	tx[12] = state_byte;

	// 1. Pull Chip Select LOW to start the transaction
	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_RESET);

	// 2. Transmit the frame and capture the return status
	spi_status = HAL_SPI_Transmit(&hspi1, tx, 13, HAL_MAX_DELAY);

	// 3. Pull Chip Select HIGH to end the transaction
	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_SET);

	return spi_status;
}
