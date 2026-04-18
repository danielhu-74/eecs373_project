#ifndef DDR_PAD_H_
#define DDR_PAD_H_

#include "game_types.h"

void delay_us(uint32_t us);
uint8_t PS2_Exchange(uint8_t data);
uint8_t PS2_SwapByte_Pad2(uint8_t data);
void PS2_ReadPad(void);
void process_Pad(Player *player);
void process_Pad_P2(Player *player);





#endif
