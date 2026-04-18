#include "game_types.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern uint8_t nunchuk_data1[6];
extern uint8_t nunchuk_data2[6];
static uint16_t last_acc_z1 = 712; // 初始值给个大概就行，运行一次后就会自动同步
static uint16_t last_acc_z2 = 712; // 初始值给个大概就行，运行一次后就会自动同步
static int cooldown_timer1 = 0;
static int cooldown_timer2 = 0;


// 差值阈值：这决定了你需要多“猛”的发力才能触发。
// 因为差值过滤了重力，这个值通常可以设小一点，比如 40 ~ 80
const int SWING_THRESHOLD = 60;
const int COOLDOWN_CYCLES = 10;

void nunchuk_init(void) {
    uint8_t init_data1[2] = {0xF0, 0x55};
    uint8_t init_data2[2] = {0xFB, 0x00};

    // 发送初始化指令
    HAL_I2C_Master_Transmit(&hi2c1, NUNCHUK_ADDR, init_data1, 2, 100);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c1, NUNCHUK_ADDR, init_data2, 2, 100);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c2, NUNCHUK_ADDR, init_data1, 2, 100);
    HAL_Delay(10);
    HAL_I2C_Master_Transmit(&hi2c2, NUNCHUK_ADDR, init_data2, 2, 100);
    HAL_Delay(10);
}





// 2. 读取 6 字节数据
void nunchuk_read_p1(void) {
    uint8_t request_data = 0x00;

    // 每次读取前，都要向手柄写入一个 0x00，告诉它“把数据指针指回开头”
    HAL_I2C_Master_Transmit(&hi2c1, NUNCHUK_ADDR, &request_data, 1, 100);

    // 给手柄一点点时间准备数据
    HAL_Delay(2);

    // 连续读取 6 个字节
    HAL_I2C_Master_Receive(&hi2c1, NUNCHUK_ADDR, nunchuk_data1, 6, 100);
}

// 2. 读取 P2 的数据
void nunchuk_read_p2(void) {
    uint8_t request_data = 0x00;

    HAL_StatusTypeDef status;

	// 1. Check the pointer reset
	status = HAL_I2C_Master_Transmit(&hi2c2, NUNCHUK_ADDR, &request_data, 1, 100);
	if (status != HAL_OK) {
		printf("P2 Transmit Error: %d\r\n", status);
		return; // Stop here if we can't even talk to it
	}

	HAL_Delay(2);

	// 2. Check the data receive
	status = HAL_I2C_Master_Receive(&hi2c2, NUNCHUK_ADDR, nunchuk_data2, 6, 100);
	if (status != HAL_OK) {
		printf("P2 Receive Error: %d\r\n", status);
	}
}

void process_nunchuk_p1(Player *player){

    nunchuk_read_p1();

    uint16_t accel_z = (nunchuk_data1[4] << 2) | ((nunchuk_data1[5] >> 6) & 0x03);

    // ---------------------------------------------------------
    // 3. 动作识别 (瞬间速度差 Delta)
    // ---------------------------------------------------------
    // 计算当前时刻与上一个时刻的差值 (极其关键：这是动态加速度，去除了重力影响)
    int16_t dz = accel_z - last_acc_z1;
    int16_t abs_dz = abs(dz);
    char current_action[50] = "Preparing";

    if (cooldown_timer1 > 0) {
        cooldown_timer1--;
        if (cooldown_timer1 == 0) {
            sprintf(current_action, "Still");
            player->swing = false;
        }
    }
    else {
        // 只有瞬间的变化量极大，才认为是挥动
        if (abs_dz > SWING_THRESHOLD) {
            if (dz > SWING_THRESHOLD) {
                sprintf(current_action, "Moving up");
                cooldown_timer1 = COOLDOWN_CYCLES;
                player->swing = true;
                if (player->side == SIDE_LEFT){
                    player->mx = HITTING_SPEED_X;
                }
                else{
                    player->mx = -HITTING_SPEED_X;
                }     
                player->my = HITTING_SPEED_Y;
            }
            else if (dz < -SWING_THRESHOLD) {
                sprintf(current_action, "Moving down");
                cooldown_timer1 = COOLDOWN_CYCLES;
//                player->swing = true;
//                if (player->side == SIDE_RIGHT){
//                    player->mx = 0.707;
//                }
//                else{
//                    player->mx = -0.707;
//                }
//                player->my = -0.707;
            }
        }
    }

    // 必须更新：把现在的状态变成“过去”，给下一次循环使用
    last_acc_z1 = accel_z;

    // 4. 打印输出
   //printf("dz:%5d | Action: %s \r\n", dz, current_action);

//    HAL_Delay(1);

}

void process_nunchuk_p2(Player *player){

    nunchuk_read_p2();

    uint16_t accel_z = (nunchuk_data2[4] << 2) | ((nunchuk_data2[5] >> 6) & 0x03);

    // ---------------------------------------------------------
    // 3. 动作识别 (瞬间速度差 Delta)
    // ---------------------------------------------------------
    // 计算当前时刻与上一个时刻的差值 (极其关键：这是动态加速度，去除了重力影响)
    int16_t dz = accel_z - last_acc_z2;
    int16_t abs_dz = abs(dz);
    char current_action[50] = "Preparing";

    if (cooldown_timer2 > 0) {
        cooldown_timer2--;
        if (cooldown_timer2 == 0) {
            sprintf(current_action, "Still");
            player->swing = false;
        }
    }
    else {
        // 只有瞬间的变化量极大，才认为是挥动
        if (abs_dz > SWING_THRESHOLD) {
            if (dz > SWING_THRESHOLD) {
                sprintf(current_action, "Moving up");
                cooldown_timer2 = COOLDOWN_CYCLES;
                player->swing = true;
                if (player->side == SIDE_LEFT){
                    player->mx = HITTING_SPEED_X;
                }
                else{
                    player->mx = -HITTING_SPEED_X;
                }
                player->my = HITTING_SPEED_Y;
            }
            else if (dz < -SWING_THRESHOLD) {
                sprintf(current_action, "Moving down");
                cooldown_timer2 = COOLDOWN_CYCLES;
                //player->swing = true;
//                if (player->side == SIDE_RIGHT){
//                    player->mx = 0.707;
//                }
//                else{
//                    player->mx = -0.707;
//                }
                player->my = -0.707;
            }
        }
    }

    // 必须更新：把现在的状态变成“过去”，给下一次循环使用
    last_acc_z2 = accel_z;

    // 4. 打印输出
    printf("dz:%5d | Action: %s \r\n", dz, current_action);

//    HAL_Delay(1);

}

