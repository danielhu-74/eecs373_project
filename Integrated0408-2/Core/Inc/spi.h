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

#define SPI_DISPLAY_MODE_BLANK  0U
#define SPI_DISPLAY_MODE_PLAY   1U
#define SPI_DISPLAY_MODE_PAUSE  2U

HAL_StatusTypeDef SPI_SendGameState(PlayerData p1,
                                    PlayerData p2,
                                    ShuttlecockData sc,
                                    uint8_t display_mode);


#endif /* INC_SPI_H_ */
