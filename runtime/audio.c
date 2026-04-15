#include "audio.h"

#include <gba_sound.h>
#include <gba_types.h>

/*
    Real but simple GBA PSG tone support.

    Current support:
    - audio_tone(freq, duration): square wave on channel 1
    - audio_update(): decrements a frame timer and stops tone automatically
    - audio_play_score(): stub for now
*/

static int g_tone_frames_remaining = 0;
static int g_tone_active = 0;

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/*
    DMG square channels use:
      frequency = 131072 / (2048 - n)
      => n = 2048 - 131072 / frequency
*/
static int gba_square_reg_from_hz(int hz) {
    int n;

    hz = clamp_int(hz, 64, 20000);
    n = 2048 - (131072 / hz);
    return clamp_int(n, 0, 2047);
}

void audio_init(void) {
    /*
        Master sound enable.
        Bit 7 of SOUNDCNT_X enables sound hardware.
    */
    REG_SOUNDCNT_X = 0x0080;

    /*
        SOUNDCNT_L:
        - bits 0-2: right volume 0-7
        - bits 4-6: left volume 0-7
        - bits 8-11: route DMG channels to right
        - bits 12-15: route DMG channels to left

        Here:
        - left volume = 7
        - right volume = 7
        - route channel 1 to both left and right
    */
    REG_SOUNDCNT_L = 0x1177;

    /*
        SOUNDCNT_H not needed for PSG-only sound.
        Leave direct sound disabled.
    */
    REG_SOUNDCNT_H = 0x0000;

    /*
        Silence channel 1 initially.
    */
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0x0000;
    REG_SOUND1CNT_X = 0x0000;

    g_tone_frames_remaining = 0;
    g_tone_active = 0;
}

void audio_stop_tone(void) {
    REG_SOUND1CNT_H = 0x0000;
    REG_SOUND1CNT_X = 0x0000;
    g_tone_frames_remaining = 0;
    g_tone_active = 0;
}

void audio_tone(int freq, int duration) {
    int reg_freq;
    int frames;

    if (freq <= 0 || duration <= 0) {
        audio_stop_tone();
        return;
    }

    reg_freq = gba_square_reg_from_hz(freq);

    /*
        SOUND1CNT_L: no sweep
    */
    REG_SOUND1CNT_L = 0x0000;

    /*
        SOUND1CNT_H layout:
        bits 6-7  : duty
        bits 8-10 : length
        bits 12-15: initial envelope volume
        bit 11    : envelope direction
        bits 8-10 / low bits not critical here for simple beeps

        0xF080:
        - duty 50%
        - initial volume 15
        - fixed envelope
    */
    REG_SOUND1CNT_H = 0xF080;

    /*
        SOUND1CNT_X:
        bit 15    : restart
        bits 0-10 : frequency value
    */
    REG_SOUND1CNT_X = (u16)(0x8000 | (reg_freq & 0x07FF));

    frames = (duration + 16) / 17;
    if (frames < 1) {
        frames = 1;
    }

    g_tone_frames_remaining = frames;
    g_tone_active = 1;
}

void audio_update(void) {
    if (!g_tone_active) {
        return;
    }

    g_tone_frames_remaining--;

    if (g_tone_frames_remaining <= 0) {
        audio_stop_tone();
    }
}

void audio_play_score(const unsigned char* score) {
    (void)score;
    /* Playtune score support can be added later */
}

void audio_stop_score(void) {
    audio_stop_tone();
}
