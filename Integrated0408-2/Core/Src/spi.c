/*
 * spi.c
 *
 *  Created on: Apr 7, 2026
 *      Author: jhsbzd
 */

#include "game_types.h"
#include "spi.h"
#include "main.h"
#include <stdio.h>


#define FPGA_CS_Pin GPIO_PIN_14
#define FPGA_CS_GPIO_Port GPIOD

extern SPI_HandleTypeDef hspi1;

uint8_t tx[13];

//static uint8_t calc_checksum(const uint8_t *data, uint16_t len) {
//    uint8_t sum = 0;
//    for (uint16_t i = 0; i < len; i++) sum ^= data[i];
//    return sum;
//}

// ─── Main Transfer Function ─────────────────────────────────────────
HAL_StatusTypeDef SPI_SendGameState(PlayerData p1, PlayerData p2, ShuttlecockData sc) {
//    SPIData frame;

    // Header
//    frame.header[0] = 0xFF;
//    frame.header[1] = 0xAA;
//
//    // Copy data
//    frame.player1     = p1;
//    frame.player2     = p2;
//    frame.shuttlecock = sc;
//
//    // Checksum over everything except header and checksum itself
//    frame.checksum = calc_checksum(
//        (uint8_t*)&frame.player1,
//        sizeof(PlayerData)*2 + sizeof(ShuttlecockData)
//    );


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
	tx[12] = (p1.swing) | (p2.swing << 1);


//    printf("%d ",tx[0]);
//    printf("%d ",tx[1]);
//    printf("%d ",tx[2]);
//    printf("%d ",tx[3]);
//    printf("%d ",tx[4]);
//    printf("%d ",tx[5]);
//    printf("%d ",tx[6]);
//    printf("%d ",tx[7]);
//    printf("%d ",tx[8]);
//	printf("%d ",tx[9]);
//	printf("%d ",tx[10]);
//	printf("%d ",tx[11]);
//    printf("%d\r\n",tx[12]);

//	printf("Left player x: %f:", p->x);

    // Transmit
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS LOW
//    HAL_StatusTypeDef status = HAL_SPI_Transmit(
//        &hspi1,
//        (uint8_t*)&frame,
//        sizeof(SPIData),
//        HAL_MAX_DELAY
//    );
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // CS HIGH

	HAL_StatusTypeDef spi_status;

	// 1. Pull Chip Select LOW to start the transaction
	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_RESET);

	// 2. Transmit the 12 bytes and capture the return status
	spi_status = HAL_SPI_Transmit(&hspi1, tx, 13, HAL_MAX_DELAY);

	// 3. Pull Chip Select HIGH to end the transaction
	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_SET);

	// 4. Check if the transmission was successful
	if (spi_status == HAL_OK) {
	    // SUCCESS: The STM32 successfully clocked out all 12 bytes.

	    // Example A: Toggle a status LED on your STM32 board
	    // HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	    // Example B: Print to a serial monitor (if you have UART setup)
	    printf("SPI Tx OK!\r\n");
	}
	else {
	    // FAILURE: Handle HAL_ERROR, HAL_BUSY, or HAL_TIMEOUT
	    printf("SPI Tx FAILED! Error code: %d\r\n", spi_status);
	}

	return spi_status;

}

