#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gba_types.h>

void gfx_init();
void gfx_clear();
void gfx_present();

void gfx_draw_pixel(int x, int y, u16 color);

void gfx_draw_fast_hline(int x, int y, int w, u16 color);
void gfx_draw_fast_vline(int x, int y, int h, u16 color);

void gfx_draw_rect(int x, int y, int w, int h, u16 color);
void gfx_fill_rect(int x, int y, int w, int h, u16 color);

void gfx_fill_screen(u16 color);

/* Arduboy bitmap format: bit-packed, vertical bytes, explicit w/h */
void gfx_draw_bitmap(int x, int y, const unsigned char* bmp, int w, int h);

/* Common Arduboy sprite-sheet formats */
void gfx_draw_sprite_overwrite(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_self_masked(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_erase(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_plus_mask(int x, int y, const unsigned char* sprite, int frame);
void gfx_draw_sprite_external_mask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame);

void gfx_set_cursor(int x, int y);
void gfx_write_char(char c);
void gfx_print(const char* str);

#endif
