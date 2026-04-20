#include "ddr_pad.h"
#include "wii_nunchuk.h"
#define PS2_DATA_PIN  PS2_DATA_Pin
#define PS2_DATA_PORT PS2_DATA_GPIO_Port
#define PS2_CMD_PIN   PS2_CMD_Pin
#define PS2_CMD_PORT  PS2_CMD_GPIO_Port
#define PS2_ATTN_PIN  PS2_ATTN_Pin
#define PS2_ATTN_PORT PS2_ATTN_GPIO_Port
#define PS2_CLK_PIN   PS2_CLK_Pin
#define PS2_CLK_PORT  PS2_CLK_GPIO_Port
#include "main.h"
extern uint8_t ps2_data[6];
extern uint8_t player2_data[6];

void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    while (DWT->CYCCNT - startTick < delayTicks);
}


uint8_t PS2_Exchange(uint8_t data) {
    uint8_t receive = 0;
    for (int i = 0; i < 8; i++) {

        if (data & (1 << i))
            HAL_GPIO_WritePin(PS2_CMD_PORT, PS2_CMD_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(PS2_CMD_PORT, PS2_CMD_PIN, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(PS2_CLK_PORT, PS2_CLK_PIN, GPIO_PIN_RESET);
        delay_us(15);


        if (HAL_GPIO_ReadPin(PS2_DATA_PORT, PS2_DATA_PIN) == GPIO_PIN_SET) {
            receive |= (1 << i);
        }

        HAL_GPIO_WritePin(PS2_CLK_PORT, PS2_CLK_PIN, GPIO_PIN_SET);
        delay_us(15);
    }
    return receive;
}


uint8_t PS2_SwapByte_Pad2(uint8_t data) {
    uint8_t received = 0;
    for(int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(GPIOE, DDR2_CMD_Pin, (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        HAL_GPIO_WritePin(GPIOE, DDR2_CLK_Pin, GPIO_PIN_RESET);
        delay_us(15);

        if(HAL_GPIO_ReadPin(GPIOF, DDR2_DAT_Pin) == GPIO_PIN_SET) {
            received |= (1 << i);
        }

        HAL_GPIO_WritePin(GPIOE, DDR2_CLK_Pin, GPIO_PIN_SET);
        delay_us(15);
    }
    return received;
}


void PS2_ReadPad() {
	HAL_GPIO_WritePin(PS2_ATTN_GPIO_Port, PS2_ATTN_Pin, GPIO_PIN_RESET);
	delay_us(20);

	PS2_Exchange(0x01);
	PS2_Exchange(0x42);
	PS2_Exchange(0x00);
	ps2_data[0] = PS2_Exchange(0x00);
	ps2_data[1] = PS2_Exchange(0x00);

	HAL_GPIO_WritePin(PS2_ATTN_GPIO_Port, PS2_ATTN_Pin, GPIO_PIN_SET);
}


void PS2_ReadPad_P2() {
	HAL_GPIO_WritePin(DDR2_ATT_GPIO_Port, DDR2_ATT_Pin, GPIO_PIN_RESET);
	delay_us(20);

	PS2_SwapByte_Pad2(0x01);
	PS2_SwapByte_Pad2(0x42);
	PS2_SwapByte_Pad2(0x00);
    player2_data[0] = PS2_SwapByte_Pad2(0x00);
    player2_data[1] = PS2_SwapByte_Pad2(0x00);

	HAL_GPIO_WritePin(DDR2_ATT_GPIO_Port, DDR2_ATT_Pin, GPIO_PIN_SET);
}

void process_Pad(Player *player){
    PS2_ReadPad();

    uint8_t btn_data1 = ~ps2_data[0];
    uint8_t up    = (btn_data1 >> 4) & 0x01;
    uint8_t down  = (btn_data1 >> 6) & 0x01;

    if (player->side == SIDE_LEFT){
        if (up) {
            player->move_right = 1;
            player->move_left = 0;
        }
        else if (down) {
            player->move_right = 0;
            player->move_left = 1;
        }
        else{
            player->move_right = 0;
            player->move_left = 0;
        }
    }
    else{
        if (up) {
            player->move_right = 0;
            player->move_left = 1;
        }
        else if (down) {
            player->move_right = 1;
            player->move_left = 0;
        }
        else{
            player->move_right = 0;
            player->move_left = 0;
        }
    }
}

void process_Pad_P2(Player *player){
    PS2_ReadPad_P2();

    uint8_t btn_data1 = ~player2_data[0];
    uint8_t up    = (btn_data1 >> 4) & 0x01;
    uint8_t down  = (btn_data1 >> 6) & 0x01;

    if (player->side == SIDE_LEFT){
        if (up) {
            player->move_right = 1;
            player->move_left = 0;
        }
        else if (down) {
            player->move_right = 0;
            player->move_left = 1;
        }
        else{
            player->move_right = 0;
            player->move_left = 0;
        }
    }
    else{
        if (up) {
            player->move_right = 0;
            player->move_left = 1;
        }
        else if (down) {
            player->move_right = 1;
            player->move_left = 0;
        }
        else{
            player->move_right = 0;
            player->move_left = 0;
        }
    }
}
