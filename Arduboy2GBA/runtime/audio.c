#include "audio.h"
#include <gba_sound.h>
#include <stdbool.h>

static const unsigned char* score = 0;
static int pos = 0;
static int wait = 0;
static bool playing = false;

static void audio_hw_init(void) {
    /* Master sound enable */
    REG_SOUNDCNT_X = 0x0080;

    /* Route PSG channels to both speakers, full volume */
    REG_SOUNDCNT_L = 0x1177;

    /* Basic PSG/FIFO mix setup */
    REG_SOUNDCNT_H = 0x000B;
}

void audio_tone(int freq, int duration) {
    if (freq <= 0 || duration <= 0) return;

    audio_hw_init();

    int period = 2048 - (131072 / freq);
    if (period < 0) period = 0;
    if (period > 2047) period = 2047;

    /*
      SOUND1:
      sweep off
      duty/envelope set to something audible
      frequency + initial trigger
    */
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0x80F0;                 /* duty/envelope */
    REG_SOUND1CNT_X = 0x8000 | (period & 0x07FF);

    for (volatile int i = 0; i < duration * 500; i++) {
        __asm__ volatile("nop");
    }

    REG_SOUND1CNT_X = 0;
}

void audio_play_score(const unsigned char* s) {
    score = s;
    pos = 0;
    wait = 0;
    playing = (s != 0);
}

void audio_stop_score() {
    playing = false;
    score = 0;
    REG_SOUND1CNT_X = 0;
}

void audio_update() {
    if (!playing || !score) return;

    if (wait > 0) {
        wait--;
        return;
    }

    unsigned char cmd = score[pos++];

    if (cmd == 0xFF) {
        audio_stop_score();
        return;
    }

    int note = score[pos++];
    int dur = score[pos++];

    int freq = 220 + note * 8;
    audio_tone(freq, dur);
    wait = dur * 2;
}
