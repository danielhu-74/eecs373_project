#ifndef __MP3_CONTROL_H
#define __MP3_CONTROL_H

#include "main.h" // 包含HAL库定义

// 声明函数，让其他文件可以调用
void Play_BGM(uint8_t folder, uint8_t file);
void Play_SFX(uint8_t folder, uint8_t file);
void Manual_MP3_INT_Init(void);
void MP3_ProcessPlaybackState(void);

// 如果其他文件需要判断播放状态，可以声明这些变量为全局可见
extern volatile uint8_t mp3_finished_flag;
extern uint8_t is_playing_sfx;

#endif
