#define ARDUBOY_COMPAT_IMPLEMENTATION
#include "arduboy_compat.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <gba_systemcalls.h>

#define GBA_SRAM_BASE ((volatile unsigned char*)0x0E000000)
#define GBA_SRAM_SIZE 0x10000

__attribute__((used))
static const char g_gba_save_type_string[] = "SRAM_V113";

static int g_frame_rate = 60;
static int g_frame_duration_vblanks = 1;
static int g_frame_counter = 0;
static int g_frame_count = 0;
static float g_time_scale = 1.0f;
static bool g_audio_enabled = true;
static bool g_input_polled_this_frame = false;

static u8 g_led_r = 0;
static u8 g_led_g = 0;
static u8 g_led_b = 0;

static uint32_t g_rng_state = 0xA341316Cu;

Arduboy2Base arduboy;
EEPROMClass EEPROM;

static void sync_led_state(void) {
    gfx_set_rgb_led(g_led_r, g_led_g, g_led_b);
}

static void set_led_component(u8 color, u8 value) {
    if (color & RED_LED) {
        g_led_r = value;
    }
    if (color & GREEN_LED) {
        g_led_g = value;
    }
    if (color & BLUE_LED) {
        g_led_b = value;
    }
}

static void print_unsigned_base(unsigned int v, int base) {
    char buf[33];

    if (base == HEX) {
        snprintf(buf, sizeof(buf), "%X", v);
        gfx_write_string(buf);
        return;
    }

    snprintf(buf, sizeof(buf), "%u", v);
    gfx_write_string(buf);
}

static uint32_t xorshift32(void) {
    uint32_t x = g_rng_state;
    if (x == 0) {
        x = 0x6D2B79F5u;
    }
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng_state = x;
    return x;
}

unsigned char EEPROMClass::read(int address) const {
    if (address < 0 || address >= GBA_SRAM_SIZE) {
        return 0;
    }
    return GBA_SRAM_BASE[address];
}

void EEPROMClass::write(int address, unsigned char value) {
    if (address < 0 || address >= GBA_SRAM_SIZE) {
        return;
    }
    GBA_SRAM_BASE[address] = value;
}

void EEPROMClass::update(int address, unsigned char value) {
    if (address < 0 || address >= GBA_SRAM_SIZE) {
        return;
    }
    if (GBA_SRAM_BASE[address] != value) {
        GBA_SRAM_BASE[address] = value;
    }
}

void ab_setTimeScale(float scale) {
    if (scale <= 0.0f) {
        scale = 1.0f;
    }
    g_time_scale = scale;
}

float ab_getTimeScale(void) {
    return g_time_scale;
}

void ab_begin(void) {
    gfx_init();
    audio_init();
    input_poll();
    g_input_polled_this_frame = true;
    g_frame_rate = 60;
    g_frame_duration_vblanks = 1;
    g_frame_counter = 0;
    g_frame_count = 0;
    g_audio_enabled = true;

    g_led_r = 0;
    g_led_g = 0;
    g_led_b = 0;
    sync_led_state();
}

void ab_beginNoLogo(void) {
    ab_begin();
}

void ab_bootLogoSpritesSelfMasked(void) {
    /* no-op for now */
}

void ab_initRandomSeed(void) {
    g_rng_state = 0xA341316Cu ^ (uint32_t)(g_frame_count * 1103515245u + 12345u);
    if (g_rng_state == 0) {
        g_rng_state = 1;
    }
}

void ab_clear(void) {
    gfx_clear();
}

void ab_display(void) {
    gfx_present();
}

void ab_display(int clear_buffer) {
    gfx_present();
    if (clear_buffer) {
        gfx_clear();
    }
}

void ab_invert(bool enable) {
    gfx_set_invert(enable);
}

void ab_drawPixel(int x, int y, int c) {
    gfx_draw_pixel(x, y, c);
}

void ab_drawFastHLine(int x, int y, int w, int c) {
    gfx_draw_fast_hline(x, y, w, c);
}

void ab_drawFastVLine(int x, int y, int h, int c) {
    gfx_draw_fast_vline(x, y, h, c);
}

void ab_drawRect(int x, int y, int w, int h, int c) {
    gfx_draw_rect(x, y, w, h, c);
}

void ab_fillRect(int x, int y, int w, int h, int c) {
    gfx_fill_rect(x, y, w, h, c);
}

void ab_fillScreen(int c) {
    gfx_fill_screen(c);
}

void ab_drawCircle(int x, int y, int r, int c) {
    gfx_draw_circle(x, y, r, c);
}

void ab_fillCircle(int x, int y, int r, int c) {
    gfx_fill_circle(x, y, r, c);
}

void ab_drawLine(int x0, int y0, int x1, int y1, int c) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        ab_drawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        {
            int e2 = err << 1;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

void ab_drawRoundRect(int x, int y, int w, int h, int r, int c) {
    gfx_draw_round_rect(x, y, w, h, r, c);
}

void ab_drawBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c) {
    (void)c;
    gfx_draw_bitmap(x, y, bmp, w, h);
}

void ab_drawOverwrite(int x, int y, const unsigned char* sprite, int frame) {
    gfx_draw_sprite_overwrite(x, y, sprite, frame);
}

void ab_drawSelfMasked(int x, int y, const unsigned char* sprite, int frame) {
    gfx_draw_sprite_self_masked(x, y, sprite, frame);
}

void ab_drawErase(int x, int y, const unsigned char* sprite, int frame) {
    gfx_draw_sprite_erase(x, y, sprite, frame);
}

void ab_drawPlusMask(int x, int y, const unsigned char* sprite, int frame) {
    gfx_draw_sprite_plus_mask(x, y, sprite, frame);
}

void ab_drawExternalMask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame) {
    gfx_draw_sprite_external_mask(x, y, sprite, mask, frame, mask_frame);
}

void ab_setCursor(int x, int y) {
    gfx_set_cursor(x, y);
}

void ab_setTextSize(int s) {
    gfx_set_text_size(s);
}

void ab_setTextWrap(bool w) {
    gfx_set_text_wrap(w);
}

bool ab_getTextWrap(void) {
    return gfx_get_text_wrap();
}

void ab_setTextRawMode(bool r) {
    gfx_set_text_raw(r);
}

bool ab_getTextRawMode(void) {
    return gfx_get_text_raw();
}

void ab_setTextColor(int c) {
    gfx_set_text_color(c);
}

void ab_setTextBackground(int c) {
    gfx_set_text_bg(c);
}

void ab_print(const char* s) {
    gfx_write_string(s);
}

void ab_print(int v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    gfx_write_string(buf);
}

void ab_print(unsigned int v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", v);
    gfx_write_string(buf);
}

void ab_print(char c) {
    gfx_write_char(c);
}

void ab_print(float v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", v);
    gfx_write_string(buf);
}

void ab_print(int v, int base) {
    if (base == HEX) {
        print_unsigned_base((unsigned int)v, base);
    } else {
        ab_print(v);
    }
}

void ab_print(unsigned int v, int base) {
    print_unsigned_base(v, base);
}

void ab_print(unsigned char v, int base) {
    print_unsigned_base((unsigned int)v, base);
}

void ab_println(void) {
    gfx_write_char('\n');
}

void ab_println(const char* s) {
    ab_print(s);
    ab_println();
}

void ab_println(int v) {
    ab_print(v);
    ab_println();
}

void ab_println(char c) {
    ab_print(c);
    ab_println();
}

void ab_println(float v) {
    ab_print(v);
    ab_println();
}

void ab_pollButtons(void) {
    if (!g_input_polled_this_frame) {
        input_poll();
        g_input_polled_this_frame = true;
    }
}

bool ab_pressed(u16 key) {
    return input_pressed(key);
}

bool ab_justPressed(u16 key) {
    return input_just_pressed(key);
}

bool ab_notPressed(u16 key) {
    return !ab_pressed(key);
}

void ab_setFrameRate(int fps) {
    int scaled_fps;
    float vblank_frames;

    if (fps <= 0) {
        fps = 60;
    }

    scaled_fps = (int)((float)fps / g_time_scale + 0.5f);

    if (scaled_fps < 1) {
        scaled_fps = 1;
    }
    if (scaled_fps > 60) {
        scaled_fps = 60;
    }

    g_frame_rate = scaled_fps;

    vblank_frames = 60.0f / (float)scaled_fps;
    g_frame_duration_vblanks = (int)(vblank_frames + 0.5f);
    if (g_frame_duration_vblanks < 1) {
        g_frame_duration_vblanks = 1;
    }

    g_frame_counter = 0;
}

bool ab_nextFrame(void) {
    g_frame_counter++;

    if (g_frame_counter >= g_frame_duration_vblanks) {
        g_frame_counter = 0;
        g_input_polled_this_frame = false;
        VBlankIntrWait();
        audio_update();
        input_poll();
        g_input_polled_this_frame = true;
        g_frame_count++;
        return true;
    }

    VBlankIntrWait();
    audio_update();
    return false;
}

bool ab_everyXFrames(int frames) {
    if (frames <= 0) {
        return false;
    }
    return (g_frame_count % frames) == 0;
}

void ab_delay(int ms) {
    int scaled_ms;
    int frames;
    int i;

    if (ms <= 0) {
        return;
    }

    scaled_ms = (int)((float)ms * g_time_scale + 0.5f);
    if (scaled_ms < 1) {
        scaled_ms = 1;
    }

    frames = (scaled_ms + 16) / 17;
    if (frames < 1) {
        frames = 1;
    }

    for (i = 0; i < frames; i++) {
        VBlankIntrWait();
        audio_update();
    }

    g_input_polled_this_frame = false;
}

void ab_idle(void) {
    VBlankIntrWait();
    audio_update();
    input_poll();
    g_input_polled_this_frame = true;
}

int ab_random(int max) {
    if (max <= 0) {
        return 0;
    }
    return (int)(xorshift32() % (uint32_t)max);
}

int ab_random(int min, int max) {
    if (max <= min) {
        return min;
    }
    return min + (int)(xorshift32() % (uint32_t)(max - min));
}

float ab_radians(float deg) {
    return deg * (3.14159265f / 180.0f);
}

void ab_tone(int freq, int duration) {
    if (!g_audio_enabled) {
        return;
    }
    audio_tone(freq, duration);
}

void ab_noTone(void) {
    audio_stop_tone();
}

void ab_playScore(const unsigned char* score) {
    if (!g_audio_enabled) {
        return;
    }
    audio_play_score(score);
}

void ab_stopScore(void) {
    audio_stop_score();
}

bool ab_audio_enabled(void) {
    return g_audio_enabled;
}

void ab_audio_on(void) {
    g_audio_enabled = true;
}

void ab_audio_off(void) {
    g_audio_enabled = false;
    audio_stop_tone();
    audio_stop_score();
}

bool ab_score_playing(void) {
    return audio_score_playing();
}

void ab_set_tone_mutes_score(bool mute) {
    audio_set_tone_mutes_score(mute);
}

void ab_setRGBled(u8 red, u8 green, u8 blue) {
    g_led_r = red;
    g_led_g = green;
    g_led_b = blue;
    sync_led_state();
}

void ab_setRGBled(u8 color, u8 value) {
    set_led_component(color, value);
    sync_led_state();
}

void ab_digitalWriteRGB(u8 color, u8 value) {
    set_led_component(color, value ? 255 : 0);
    sync_led_state();
}

void ab_digitalWriteRGB(u8 red, u8 green, u8 blue) {
    g_led_r = red ? 255 : 0;
    g_led_g = green ? 255 : 0;
    g_led_b = blue ? 255 : 0;
    sync_led_state();
}

void ab_freeRGBled(void) {
}

int ab_get_frame_duration_ms(void) {
    return (g_frame_rate > 0) ? (1000 / g_frame_rate) : 16;
}

int ab_get_dropped_frames(void) {
    return 0;
}

int ab_get_last_present_ticks(void) {
    return (int)gfx_get_last_present_ticks();
}

int ab_get_max_present_ticks(void) {
    return (int)gfx_get_max_present_ticks();
}
