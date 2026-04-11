#include "arduboy_compat.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"

void ab_init() { gfx_init(); }
void ab_clear() { gfx_clear(); }
void ab_display() { gfx_present(); }

void ab_drawPixel(int x, int y, u16 c) { gfx_draw_pixel(x, y, c); }
void ab_drawFastHLine(int x, int y, int w, u16 c) { gfx_draw_fast_hline(x, y, w, c); }
void ab_drawFastVLine(int x, int y, int h, u16 c) { gfx_draw_fast_vline(x, y, h, c); }

void ab_drawRect(int x, int y, int w, int h, u16 c) { gfx_draw_rect(x, y, w, h, c); }
void ab_fillRect(int x, int y, int w, int h, u16 c) { gfx_fill_rect(x, y, w, h, c); }
void ab_fillScreen(u16 c) { gfx_fill_screen(c); }

void ab_drawBitmap(int x, int y, const unsigned char* b, int w, int h) {
    gfx_draw_bitmap(x, y, b, w, h);
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

void ab_setCursor(int x, int y) { gfx_set_cursor(x, y); }
void ab_print(const char* s) { gfx_print(s); }

u16 ab_pollButtons() { return input_poll(); }
int ab_pressed(u16 k) { return input_pressed(k); }

void ab_tone(int f, int d) { audio_tone(f, d); }
void ab_playScore(const unsigned char* s) { audio_play_score(s); }
void ab_stopScore() { audio_stop_score(); }
