#ifndef AUDIO_H
#define AUDIO_H

void audio_tone(int freq, int duration);
void audio_play_score(const unsigned char* score);
void audio_stop_score();
void audio_update();

#endif
