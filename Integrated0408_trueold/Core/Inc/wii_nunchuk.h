#ifndef WII_NUNCHUK_H_
#define WII_NUNCHUK_H_

#include "game_types.h"

void nunchuk_init(void);
void nunchuk_read(void);
void process_nunchuk_p1(Player *player);
void process_nunchuk_p2(Player *player);

//void Nunchuk_Init(void);
#endif 

