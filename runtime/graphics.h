#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gba_types.h>
#include <stdbool.h>

void gfx_init(void);
void gfx_clear(void);
void gfx_present(void);

void gfx_draw_pixel(int x, int y, u16 color);
void gfx_draw_fast_hline(int x, int y, int w, u16 color);
void gfx_draw_fast_vline(int x, int y, int h, u16 color);
void gfx_draw_rect(int x, int y, int w, int h, u16 color);
void gfx_fill_rect(int x, int y, int w, int h, u16 color);
void gfx_fill_screen(u16 color);
void gfx_draw_bitmap(int x, int y, const unsigned char* bmp, int w, int h);
void gfx_draw_circle(int x0, int y0, int r, u16 color);
void gfx_fill_circle(int x0, int y0, int r, u16 color);

void gfx_draw_sprite_overwrite(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_self_masked(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_erase(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_plus_mask(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_external_mask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame);

void gfx_set_cursor(int x, int y);
void gfx_set_text_size(int size);
void gfx_set_text_wrap(bool w);
bool gfx_get_text_wrap(void);
void gfx_set_text_raw(bool r);
bool gfx_get_text_raw(void);
void gfx_set_text_color(int c);
void gfx_set_text_bg(int c);
void gfx_set_invert(bool enable);

void gfx_set_rgb_led(u8 r, u8 g, u8 b);

void gfx_write_char(char c);
void gfx_write_string(const char* s);
void gfx_write_int(int v);

u32 gfx_get_last_present_ticks(void);
u32 gfx_get_max_present_ticks(void);
void gfx_reset_perf_counters(void);

#endif
