#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

void audio_init(void);
void audio_update(void);
void audio_tone(int freq, int duration);
void audio_stop_tone(void);
void audio_play_score(const unsigned char* score);
void audio_stop_score(void);

#endif
