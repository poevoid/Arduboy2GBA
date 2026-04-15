#include "audio.h"

#include <gba_sound.h>
#include <gba_types.h>
#include <stdbool.h>
#include <math.h>

#ifndef REG_SOUND1CNT_L
#define REG_SOUND1CNT_L (*(volatile u16*)0x04000060)
#endif
#ifndef REG_SOUND1CNT_H
#define REG_SOUND1CNT_H (*(volatile u16*)0x04000062)
#endif
#ifndef REG_SOUND1CNT_X
#define REG_SOUND1CNT_X (*(volatile u16*)0x04000064)
#endif
#ifndef REG_SOUND2CNT_L
#define REG_SOUND2CNT_L (*(volatile u16*)0x04000068)
#endif
#ifndef REG_SOUND2CNT_H
#define REG_SOUND2CNT_H (*(volatile u16*)0x0400006C)
#endif
#ifndef REG_SOUNDCNT_L
#define REG_SOUNDCNT_L (*(volatile u16*)0x04000080)
#endif
#ifndef REG_SOUNDCNT_H
#define REG_SOUNDCNT_H (*(volatile u16*)0x04000082)
#endif
#ifndef REG_SOUNDCNT_X
#define REG_SOUNDCNT_X (*(volatile u16*)0x04000084)
#endif

#define AUDIO_FRAME_US 16667

#define CMD_PLAYNOTE   0x90
#define CMD_STOPNOTE   0x80
#define CMD_INSTRUMENT 0xC0
#define CMD_RESTART    0xE0
#define CMD_STOP       0xF0

typedef struct {
    bool active;
    int note;
    int freq;
} ScoreChannelState;

static int g_tone_us_remaining = 0;
static int g_tone_freq = 0;
static bool g_tone_active = false;

static bool g_score_playing = false;
static const unsigned char* g_score_start = 0;
static const unsigned char* g_score_cursor = 0;
static int g_score_wait_us = 0;

static ScoreChannelState g_score_channels[2];
static bool g_tone_mutes_score = false;

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int gba_square_reg_from_hz(int hz) {
    int n;

    hz = clamp_int(hz, 64, 20000);
    n = 2048 - (131072 / hz);
    return clamp_int(n, 0, 2047);
}

static int midi_note_to_hz(int note) {
    double hz;

    if (note < 0) {
        note = 0;
    }
    if (note > 127) {
        note = 127;
    }

    hz = 440.0 * pow(2.0, ((double)note - 69.0) / 12.0);
    if (hz < 1.0) {
        hz = 1.0;
    }

    return (int)(hz + 0.5);
}

static void psg_stop_channel_1(void) {
    REG_SOUND1CNT_H = 0x0000;
    REG_SOUND1CNT_X = 0x0000;
}

static void psg_stop_channel_2(void) {
    REG_SOUND2CNT_L = 0x0000;
    REG_SOUND2CNT_H = 0x0000;
}

static void psg_play_channel_1(int hz) {
    int reg_freq;

    if (hz <= 0) {
        psg_stop_channel_1();
        return;
    }

    reg_freq = gba_square_reg_from_hz(hz);

    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0xF080;
    REG_SOUND1CNT_X = (u16)(0x8000 | (reg_freq & 0x07FF));
}

static void psg_play_channel_2(int hz) {
    int reg_freq;

    if (hz <= 0) {
        psg_stop_channel_2();
        return;
    }

    reg_freq = gba_square_reg_from_hz(hz);

    REG_SOUND2CNT_L = 0xF080;
    REG_SOUND2CNT_H = (u16)(0x8000 | (reg_freq & 0x07FF));
}

static void refresh_output_state(void) {
    int score0_freq = g_score_channels[0].active ? g_score_channels[0].freq : 0;
    int score1_freq = g_score_channels[1].active ? g_score_channels[1].freq : 0;

    if (g_tone_active) {
        if (g_tone_mutes_score) {
            psg_stop_channel_1();
        } else {
            psg_play_channel_1(score0_freq);
        }

        psg_play_channel_2(g_tone_freq);
        return;
    }

    psg_play_channel_1(score0_freq);
    psg_play_channel_2(score1_freq);
}

static void clear_score_channels(void) {
    g_score_channels[0].active = false;
    g_score_channels[0].note = 0;
    g_score_channels[0].freq = 0;

    g_score_channels[1].active = false;
    g_score_channels[1].note = 0;
    g_score_channels[1].freq = 0;
}

static void skip_optional_header_if_present(const unsigned char** score_ptr) {
    const unsigned char* p = *score_ptr;

    if (!p) {
        return;
    }

    if (p[0] == 'P' && p[1] == 't') {
        unsigned int header_len = p[2];
        if (header_len >= 3) {
            *score_ptr = p + header_len;
        }
    }
}

static void step_score_until_wait_or_stop(void) {
    while (g_score_playing && g_score_cursor) {
        unsigned char cmd = *g_score_cursor++;

        if (cmd < 0x80) {
            unsigned char lo = *g_score_cursor++;
            unsigned int wait_ms = ((unsigned int)cmd << 8) | (unsigned int)lo;

            g_score_wait_us = (int)(wait_ms * 1000U);
            if (g_score_wait_us <= 0) {
                g_score_wait_us = 1;
            }

            refresh_output_state();
            return;
        }

        switch (cmd & 0xF0) {
            case CMD_PLAYNOTE: {
                int chan = cmd & 0x0F;
                int note = (int)(*g_score_cursor++);

                if (chan >= 0 && chan < 2) {
                    g_score_channels[chan].active = true;
                    g_score_channels[chan].note = note;
                    g_score_channels[chan].freq = midi_note_to_hz(note);
                }
                break;
            }

            case CMD_STOPNOTE: {
                int chan = cmd & 0x0F;

                if (chan >= 0 && chan < 2) {
                    g_score_channels[chan].active = false;
                    g_score_channels[chan].note = 0;
                    g_score_channels[chan].freq = 0;
                }
                break;
            }

            case CMD_INSTRUMENT:
                g_score_cursor++;
                break;

            case CMD_RESTART:
                g_score_cursor = g_score_start;
                skip_optional_header_if_present(&g_score_cursor);
                break;

            case CMD_STOP:
                g_score_playing = false;
                g_score_wait_us = 0;
                clear_score_channels();
                refresh_output_state();
                return;

            default:
                break;
        }
    }
}

void audio_init(void) {
    REG_SOUNDCNT_X = 0x0080;

    /*
        Route DMG channels 1 and 2 to both left and right speakers,
        with max left/right volume.
    */
    REG_SOUNDCNT_L = 0x3377;
    REG_SOUNDCNT_H = 0x0000;

    psg_stop_channel_1();
    psg_stop_channel_2();

    g_tone_us_remaining = 0;
    g_tone_freq = 0;
    g_tone_active = false;

    g_score_playing = false;
    g_score_start = 0;
    g_score_cursor = 0;
    g_score_wait_us = 0;

    clear_score_channels();
    g_tone_mutes_score = false;
}

void audio_stop_tone(void) {
    g_tone_us_remaining = 0;
    g_tone_freq = 0;
    g_tone_active = false;
    refresh_output_state();
}

void audio_tone(int freq, int duration) {
    if (freq <= 0) {
        audio_stop_tone();
        return;
    }

    g_tone_freq = freq;
    g_tone_active = true;

    if (duration == 0) {
        g_tone_us_remaining = -1;
    } else {
        if (duration < 0) {
            duration = 0;
        }
        g_tone_us_remaining = duration * 1000;
        if (g_tone_us_remaining <= 0) {
            g_tone_us_remaining = 1;
        }
    }

    refresh_output_state();
}

void audio_play_score(const unsigned char* score) {
    if (!score) {
        audio_stop_score();
        return;
    }

    g_score_start = score;
    skip_optional_header_if_present(&g_score_start);

    g_score_cursor = g_score_start;
    g_score_playing = true;
    g_score_wait_us = 0;
    clear_score_channels();

    step_score_until_wait_or_stop();
    refresh_output_state();
}

void audio_stop_score(void) {
    g_score_playing = false;
    g_score_start = 0;
    g_score_cursor = 0;
    g_score_wait_us = 0;
    clear_score_channels();
    refresh_output_state();
}

bool audio_score_playing(void) {
    return g_score_playing;
}

void audio_set_tone_mutes_score(bool mute) {
    g_tone_mutes_score = mute;
    refresh_output_state();
}

void audio_update(void) {
    if (g_tone_active && g_tone_us_remaining > 0) {
        g_tone_us_remaining -= AUDIO_FRAME_US;
        if (g_tone_us_remaining <= 0) {
            audio_stop_tone();
        }
    }

    if (g_score_playing) {
        if (g_score_wait_us > 0) {
            g_score_wait_us -= AUDIO_FRAME_US;
        }

        while (g_score_playing && g_score_wait_us <= 0) {
            step_score_until_wait_or_stop();

            if (!g_score_playing) {
                break;
            }

            if (g_score_wait_us > 0) {
                break;
            }
        }
    }
}
