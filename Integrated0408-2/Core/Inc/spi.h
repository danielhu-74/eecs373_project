/*
 * spi.h
 *
 *  Created on: Apr 7, 2026
 *      Author: jhsbzd
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "game_types.h"
#include "main.h"

//static uint8_t calc_checksum(const uint8_t *data, uint16_t len);
HAL_StatusTypeDef SPI_SendGameState(PlayerData p1, PlayerData p2, ShuttlecockData sc);


#endif /* INC_SPI_H_ */
