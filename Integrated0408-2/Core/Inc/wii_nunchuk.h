#ifndef WII_NUNCHUK_H_
#define WII_NUNCHUK_H_

#include "game_types.h"
#include "main.h"

typedef struct
{
    uint8_t c_pressed;
    uint8_t z_pressed;
} NunchukButtonsState;

void nunchuk_init(void);
void nunchuk_read(void);
void process_nunchuk_p1(Player *player);
void process_nunchuk_p2(Player *player);
HAL_StatusTypeDef Nunchuk_ReadButtonsP1(NunchukButtonsState *state);
HAL_StatusTypeDef Nunchuk_ReadButtonsP2(NunchukButtonsState *state);

//void Nunchuk_Init(void);
#endif 
