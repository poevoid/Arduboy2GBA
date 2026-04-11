#include "graphics.h"
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <string.h>

#define WIDTH 240
#define HEIGHT 160
#define ARDU_W 128
#define ARDU_H 64

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0x7FFF

static u16* buffer = (u16*)0x6000000;
static int ox = (WIDTH - ARDU_W) / 2;
static int oy = (HEIGHT - ARDU_H) / 2;

static int cursor_x = 0;
static int cursor_y = 0;

/*
  Standard Arduino/Arduboy-style 5x7 glyphs rendered into a 6x8 cell.
  ASCII 0x20..0x7F, 5 bytes per glyph, each byte is one column, LSB at top.
*/
static const unsigned char font5x7[96][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x7F,0x41,0x41,0x00}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */
    {0x00,0x41,0x41,0x7F,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x02}, /* f */
    {0x08,0x14,0x54,0x54,0x3C}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */
    {0x7F,0x10,0x28,0x44,0x00}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */
    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */
    {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00}, /* | */
    {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x08,0x08,0x2A,0x1C,0x08}, /* ~ */
    {0x7F,0x41,0x5D,0x49,0x7F}  /* DEL fallback */
};

static int bitmap_get_bit(const unsigned char* bmp, int w, int sx, int sy) {
    int page = sy >> 3;
    int bit = sy & 7;
    int index = sx + (page * w);
    return (bmp[index] >> bit) & 1;
}

static void draw_bitmap_core(int x, int y, const unsigned char* bmp, int w, int h, int transparent) {
    int sx, sy;

    if (!bmp || w <= 0 || h <= 0) return;

    for (sy = 0; sy < h; sy++) {
        for (sx = 0; sx < w; sx++) {
            int pixel = bitmap_get_bit(bmp, w, sx, sy);
            if (transparent) {
                if (pixel) {
                    gfx_draw_pixel(x + sx, y + sy, COLOR_WHITE);
                }
            } else {
                gfx_draw_pixel(x + sx, y + sy, pixel ? COLOR_WHITE : COLOR_BLACK);
            }
        }
    }
}

static int sprite_width(const unsigned char* sprite) {
    return (int)sprite[0];
}

static int sprite_height(const unsigned char* sprite) {
    return (int)sprite[1];
}

static int sprite_pages(int h) {
    return (h + 7) >> 3;
}

static const unsigned char* sprite_frame_ptr(const unsigned char* sprite, int frame) {
    int w = sprite_width(sprite);
    int h = sprite_height(sprite);
    int bytes_per_frame = w * sprite_pages(h);
    return sprite + 2 + (frame * bytes_per_frame);
}

static const unsigned char* plus_mask_frame_ptr(const unsigned char* sprite, int frame) {
    int w = sprite_width(sprite);
    int h = sprite_height(sprite);
    int bytes_per_frame = w * sprite_pages(h) * 2;
    return sprite + 2 + (frame * bytes_per_frame);
}

void gfx_init() {
    REG_DISPCNT = MODE_3 | BG2_ENABLE;
}

void gfx_clear() {
    memset(buffer, 0, WIDTH * HEIGHT * 2);
}

void gfx_present() {
    VBlankIntrWait();
}

void gfx_draw_pixel(int x, int y, u16 color) {
    int px = x + ox;
    int py = y + oy;

    if (px < 0 || py < 0 || px >= WIDTH || py >= HEIGHT) return;
    buffer[py * WIDTH + px] = color;
}

void gfx_draw_fast_hline(int x, int y, int w, u16 color) {
    int i;
    for (i = 0; i < w; i++) {
        gfx_draw_pixel(x + i, y, color);
    }
}

void gfx_draw_fast_vline(int x, int y, int h, u16 color) {
    int i;
    for (i = 0; i < h; i++) {
        gfx_draw_pixel(x, y + i, color);
    }
}

void gfx_draw_rect(int x, int y, int w, int h, u16 color) {
    if (w <= 0 || h <= 0) return;

    gfx_draw_fast_hline(x, y, w, color);
    gfx_draw_fast_hline(x, y + h - 1, w, color);
    gfx_draw_fast_vline(x, y, h, color);
    gfx_draw_fast_vline(x + w - 1, y, h, color);
}

void gfx_fill_rect(int x, int y, int w, int h, u16 color) {
    int j;
    if (w <= 0 || h <= 0) return;

    for (j = 0; j < h; j++) {
        gfx_draw_fast_hline(x, y + j, w, color);
    }
}

void gfx_fill_screen(u16 color) {
    int i;
    for (i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i] = color;
    }
}

void gfx_draw_bitmap(int x, int y, const unsigned char* bmp, int w, int h) {
    draw_bitmap_core(x, y, bmp, w, h, 0);
}

void gfx_draw_sprite_overwrite(int x, int y, const unsigned char* sprite, int frame) {
    int w, h;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite_width(sprite);
    h = sprite_height(sprite);
    data = sprite_frame_ptr(sprite, frame);

    draw_bitmap_core(x, y, data, w, h, 0);
}

void gfx_draw_sprite_self_masked(int x, int y, const unsigned char* sprite, int frame) {
    int w, h;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite_width(sprite);
    h = sprite_height(sprite);
    data = sprite_frame_ptr(sprite, frame);

    draw_bitmap_core(x, y, data, w, h, 1);
}

void gfx_draw_sprite_erase(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, sx, sy;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite_width(sprite);
    h = sprite_height(sprite);
    data = sprite_frame_ptr(sprite, frame);

    for (sy = 0; sy < h; sy++) {
        for (sx = 0; sx < w; sx++) {
            if (bitmap_get_bit(data, w, sx, sy)) {
                gfx_draw_pixel(x + sx, y + sy, COLOR_BLACK);
            }
        }
    }
}

void gfx_draw_sprite_plus_mask(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages, sx, sy;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite_width(sprite);
    h = sprite_height(sprite);
    pages = sprite_pages(h);
    data = plus_mask_frame_ptr(sprite, frame);

    for (sy = 0; sy < h; sy++) {
        int page = sy >> 3;
        int bit = sy & 7;

        for (sx = 0; sx < w; sx++) {
            int base = (page * w + sx) * 2;
            int image_bit = (data[base] >> bit) & 1;
            int mask_bit = (data[base + 1] >> bit) & 1;

            if (mask_bit) {
                gfx_draw_pixel(x + sx, y + sy, image_bit ? COLOR_WHITE : COLOR_BLACK);
            }
        }
    }

    (void)pages;
}

void gfx_draw_sprite_external_mask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame) {
    int w, h, sx, sy;
    const unsigned char* data;
    const unsigned char* mask_data;

    if (!sprite || !mask) return;

    w = sprite_width(sprite);
    h = sprite_height(sprite);
    data = sprite_frame_ptr(sprite, frame);
    mask_data = sprite_frame_ptr(mask, mask_frame);

    for (sy = 0; sy < h; sy++) {
        for (sx = 0; sx < w; sx++) {
            int image_bit = bitmap_get_bit(data, w, sx, sy);
            int mask_bit = bitmap_get_bit(mask_data, w, sx, sy);

            if (mask_bit) {
                gfx_draw_pixel(x + sx, y + sy, image_bit ? COLOR_WHITE : COLOR_BLACK);
            }
        }
    }
}

void gfx_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void gfx_write_char(char c) {
    int glyph_index;
    int col, row;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 8;
        return;
    }

    if ((unsigned char)c < 32 || (unsigned char)c > 127) {
        c = 127;
    }

    glyph_index = ((unsigned char)c) - 32;

    for (col = 0; col < 5; col++) {
        unsigned char bits = font5x7[glyph_index][col];
        for (row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                gfx_draw_pixel(cursor_x + col, cursor_y + row, COLOR_WHITE);
            } else {
                gfx_draw_pixel(cursor_x + col, cursor_y + row, COLOR_BLACK);
            }
        }
    }

    /* 6th column spacing */
    for (row = 0; row < 8; row++) {
        gfx_draw_pixel(cursor_x + 5, cursor_y + row, COLOR_BLACK);
    }

    /* 8th row blank baseline */
    for (col = 0; col < 6; col++) {
        gfx_draw_pixel(cursor_x + col, cursor_y + 7, COLOR_BLACK);
    }

    cursor_x += 6;
}

void gfx_print(const char* str) {
    if (!str) return;

    while (*str) {
        gfx_write_char(*str++);
    }
}
