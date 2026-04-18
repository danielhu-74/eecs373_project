#include "mp3_control.h"

extern UART_HandleTypeDef huart3;

volatile uint8_t mp3_finished_flag = 0U;
uint8_t is_playing_sfx = 0U;

static uint8_t current_bgm_folder = 2U;
static uint8_t current_bgm_file = 1U;

static void MP3_Send_Cmd(uint8_t folder, uint8_t file)
{
    uint8_t cmd[8] = {0x7EU, 0xFFU, 0x06U, 0x0FU, 0x00U, folder, file, 0xEFU};
    HAL_UART_Transmit(&huart3, cmd, 8U, 100U);
}

void Manual_MP3_INT_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void Play_BGM(uint8_t folder, uint8_t file)
{
    current_bgm_folder = folder;
    current_bgm_file = file;
    is_playing_sfx = 0U;
    MP3_Send_Cmd(folder, file);
}

void Play_SFX(uint8_t folder, uint8_t file)
{
    is_playing_sfx = 1U;
    MP3_Send_Cmd(folder, file);
}

void MP3_ProcessPlaybackState(void)
{
    if (mp3_finished_flag == 0U) {
        return;
    }

    mp3_finished_flag = 0U;

    if (is_playing_sfx != 0U) {
        is_playing_sfx = 0U;
    }

    MP3_Send_Cmd(current_bgm_folder, current_bgm_file);
}
