#ifndef __MP3_CONTROL_H
#define __MP3_CONTROL_H

#include "main.h"

void Play_BGM(uint8_t folder, uint8_t file);
void Play_SFX(uint8_t folder, uint8_t file);
void Manual_MP3_INT_Init(void);
void MP3_ProcessPlaybackState(void);

extern volatile uint8_t mp3_finished_flag;
extern uint8_t is_playing_sfx;

#endif
