#include "graphics.h"
#include <gba_video.h>
#include <gba_dma.h>
#include <string.h>
#include <stdio.h>

#define GBA_WIDTH 240
#define GBA_HEIGHT 160
#define ARDU_W 128
#define ARDU_H 64

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0x7FFF

#define LED_INDICATOR_SIZE 16
#define LED_INDICATOR_RADIUS 8
#define LED_INDICATOR_SPACING 6

static volatile u16* vram = (volatile u16*)0x6000000;
static u16 backbuffer[ARDU_W * ARDU_H] __attribute__((section(".ewram"), aligned(4)));
static unsigned char dirty_rows[ARDU_H] __attribute__((section(".ewram"), aligned(4)));

static int ox = (GBA_WIDTH - ARDU_W) / 2;
static int oy = (GBA_HEIGHT - ARDU_H) / 2;

static int cursor_x = 0;
static int cursor_y = 0;
static int text_size = 1;
static bool text_wrap = true;
static bool text_raw = false;
static int text_color = 1;
static int text_bg = 0;
static bool full_redraw_needed = true;
static bool invert_enabled = false;

static u8 led_r = 0;
static u8 led_g = 0;
static u8 led_b = 0;

static const unsigned char font5x7[96][5] = {
    {0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x5F,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},
    {0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x56,0x20,0x50},
    {0x00,0x08,0x07,0x03,0x00},
    {0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00},
    {0x2A,0x1C,0x7F,0x1C,0x2A},
    {0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x80,0x70,0x30,0x00},
    {0x08,0x08,0x08,0x08,0x08},
    {0x00,0x00,0x60,0x60,0x00},
    {0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x72,0x49,0x49,0x49,0x46},
    {0x21,0x41,0x49,0x4D,0x33},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x31},
    {0x41,0x21,0x11,0x09,0x07},
    {0x36,0x49,0x49,0x49,0x36},
    {0x46,0x49,0x49,0x29,0x1E},
    {0x00,0x00,0x14,0x00,0x00},
    {0x00,0x40,0x34,0x00,0x00},
    {0x00,0x08,0x14,0x22,0x41},
    {0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08},
    {0x02,0x01,0x59,0x09,0x06},
    {0x3E,0x41,0x5D,0x59,0x4E},
    {0x7C,0x12,0x11,0x12,0x7C},
    {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x41,0x3E},
    {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x41,0x51,0x73},
    {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x1C,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46},
    {0x26,0x49,0x49,0x49,0x32},
    {0x03,0x01,0x7F,0x01,0x03},
    {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},
    {0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03},
    {0x61,0x59,0x49,0x4D,0x43},
    {0x00,0x7F,0x41,0x41,0x41},
    {0x02,0x04,0x08,0x10,0x20},
    {0x41,0x41,0x41,0x7F,0x00},
    {0x04,0x02,0x01,0x02,0x04},
    {0x40,0x40,0x40,0x40,0x40},
    {0x00,0x03,0x07,0x08,0x00},
    {0x20,0x54,0x54,0x78,0x40},
    {0x7F,0x28,0x44,0x44,0x38},
    {0x38,0x44,0x44,0x44,0x28},
    {0x38,0x44,0x44,0x28,0x7F},
    {0x38,0x54,0x54,0x54,0x18},
    {0x00,0x08,0x7E,0x09,0x02},
    {0x18,0xA4,0xA4,0x9C,0x78},
    {0x7F,0x08,0x04,0x04,0x78},
    {0x00,0x44,0x7D,0x40,0x00},
    {0x20,0x40,0x40,0x3D,0x00},
    {0x7F,0x10,0x28,0x44,0x00},
    {0x00,0x41,0x7F,0x40,0x00},
    {0x7C,0x04,0x78,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78},
    {0x38,0x44,0x44,0x44,0x38},
    {0xFC,0x18,0x24,0x24,0x18},
    {0x18,0x24,0x24,0x18,0xFC},
    {0x7C,0x08,0x04,0x04,0x08},
    {0x48,0x54,0x54,0x54,0x24},
    {0x04,0x04,0x3F,0x44,0x24},
    {0x3C,0x40,0x40,0x20,0x7C},
    {0x1C,0x20,0x40,0x20,0x1C},
    {0x3C,0x40,0x30,0x40,0x3C},
    {0x44,0x28,0x10,0x28,0x44},
    {0x4C,0x90,0x90,0x90,0x7C},
    {0x44,0x64,0x54,0x4C,0x44},
    {0x00,0x08,0x36,0x41,0x41},
    {0x00,0x00,0x77,0x00,0x00},
    {0x41,0x41,0x36,0x08,0x00},
    {0x02,0x01,0x02,0x04,0x02},
    {0x7C,0x44,0x44,0x44,0x7C}
};

static inline u16 mono_to_color(int c) {
    int on = c ? 1 : 0;
    if (invert_enabled) {
        on = !on;
    }
    return on ? COLOR_WHITE : COLOR_BLACK;
}

static inline u16 rgb888_to_gba555(u8 r, u8 g, u8 b) {
    u16 rr = (u16)(r >> 3);
    u16 gg = (u16)(g >> 3);
    u16 bb = (u16)(b >> 3);
    return (u16)(rr | (gg << 5) | (bb << 10));
}

static inline void dma_copy_row_16(const u16* src, volatile u16* dst, int count) {
    REG_DMA3CNT = 0;
    REG_DMA3SAD = (u32)src;
    REG_DMA3DAD = (u32)dst;
    REG_DMA3CNT = DMA_ENABLE | DMA16 | count;
}

static inline void mark_row_dirty(int y) {
    if (y >= 0 && y < ARDU_H) {
        dirty_rows[y] = 1;
    }
}

static void draw_led_indicator(void) {
    int left = ox - LED_INDICATOR_SPACING - LED_INDICATOR_SIZE;
    int top = oy + (ARDU_H - LED_INDICATOR_SIZE) / 2;
    int cx = left + LED_INDICATOR_RADIUS;
    int cy = top + LED_INDICATOR_RADIUS;
    int y;

    u16 fill = rgb888_to_gba555(led_r, led_g, led_b);
    u16 border = COLOR_WHITE;

    if (invert_enabled) {
        fill ^= 0x7FFF;
        border = COLOR_BLACK;
    }

    for (y = 0; y < LED_INDICATOR_SIZE; ++y) {
        int x;
        int py = top + y;

        if (py < 0 || py >= GBA_HEIGHT) {
            continue;
        }

        for (x = 0; x < LED_INDICATOR_SIZE; ++x) {
            int px = left + x;
            int dx;
            int dy;
            int dist2;

            if (px < 0 || px >= GBA_WIDTH) {
                continue;
            }

            dx = px - cx;
            dy = py - cy;
            dist2 = dx * dx + dy * dy;

            if (dist2 <= (LED_INDICATOR_RADIUS - 1) * (LED_INDICATOR_RADIUS - 1)) {
                vram[py * GBA_WIDTH + px] = fill;
            } else if (dist2 <= LED_INDICATOR_RADIUS * LED_INDICATOR_RADIUS) {
                vram[py * GBA_WIDTH + px] = border;
            }
        }
    }
}

void gfx_init(void) {
    REG_DISPCNT = MODE_3 | BG2_ENABLE;
    gfx_clear();
}

void gfx_clear(void) {
    memset(backbuffer, 0, sizeof(backbuffer));
    memset(dirty_rows, 1, sizeof(dirty_rows));
    full_redraw_needed = true;
    cursor_x = 0;
    cursor_y = 0;
}

void gfx_present(void) {
    int y;

    for (y = 0; y < ARDU_H; y++) {
        if (full_redraw_needed || dirty_rows[y]) {
            u16* src = &backbuffer[y * ARDU_W];
            volatile u16* dst = &vram[(y + oy) * GBA_WIDTH + ox];
            dma_copy_row_16(src, dst, ARDU_W);
            dirty_rows[y] = 0;
        }
    }

    REG_DMA3CNT = 0;
    full_redraw_needed = false;

    draw_led_indicator();
}

void gfx_draw_pixel(int x, int y, u16 color) {
    if (x < 0 || y < 0 || x >= ARDU_W || y >= ARDU_H) {
        return;
    }
    backbuffer[y * ARDU_W + x] = mono_to_color(color);
    dirty_rows[y] = 1;
}

void gfx_draw_fast_hline(int x, int y, int w, u16 color) {
    int i;
    u16 mapped;

    if (y < 0 || y >= ARDU_H || w <= 0) return;
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (x + w > ARDU_W) {
        w = ARDU_W - x;
    }
    if (w <= 0) return;

    mapped = mono_to_color(color);
    for (i = 0; i < w; i++) {
        backbuffer[y * ARDU_W + x + i] = mapped;
    }
    dirty_rows[y] = 1;
}

void gfx_draw_fast_vline(int x, int y, int h, u16 color) {
    int i;
    u16 mapped;

    if (x < 0 || x >= ARDU_W || h <= 0) return;
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (y + h > ARDU_H) {
        h = ARDU_H - y;
    }
    if (h <= 0) return;

    mapped = mono_to_color(color);
    for (i = 0; i < h; i++) {
        backbuffer[(y + i) * ARDU_W + x] = mapped;
        dirty_rows[y + i] = 1;
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
    for (j = 0; j < h; j++) {
        gfx_draw_fast_hline(x, y + j, w, color);
    }
}

void gfx_fill_screen(u16 color) {
    int i;
    u16 mapped = mono_to_color(color);
    for (i = 0; i < ARDU_W * ARDU_H; i++) {
        backbuffer[i] = mapped;
    }
    memset(dirty_rows, 1, sizeof(dirty_rows));
    full_redraw_needed = true;
}

void gfx_draw_bitmap(int x, int y, const unsigned char* bmp, int w, int h) {
    int sx, sy;
    int pages;

    if (!bmp || w <= 0 || h <= 0) return;

    pages = (h + 7) / 8;

    for (sx = 0; sx < w; sx++) {
        for (sy = 0; sy < h; sy++) {
            int page = sy / 8;
            int bit = sy & 7;
            int idx = sx + page * w;
            int pixel = (bmp[idx] >> bit) & 1;
            gfx_draw_pixel(x + sx, y + sy, pixel ? 1 : 0);
        }
    }

    (void)pages;
}

void gfx_draw_circle(int x0, int y0, int r, u16 color) {
    int x = r;
    int y = 0;
    int err = 1 - x;

    while (x >= y) {
        gfx_draw_pixel(x0 + x, y0 + y, color);
        gfx_draw_pixel(x0 + y, y0 + x, color);
        gfx_draw_pixel(x0 - y, y0 + x, color);
        gfx_draw_pixel(x0 - x, y0 + y, color);
        gfx_draw_pixel(x0 - x, y0 - y, color);
        gfx_draw_pixel(x0 - y, y0 - x, color);
        gfx_draw_pixel(x0 + y, y0 - x, color);
        gfx_draw_pixel(x0 + x, y0 - y, color);

        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void gfx_fill_circle(int x0, int y0, int r, u16 color) {
    int y;

    for (y = -r; y <= r; y++) {
        int x;
        int span = 0;

        while ((span + 1) * (span + 1) + y * y <= r * r) {
            span++;
        }

        for (x = -span + 1; x <= span - 1; x++) {
            gfx_draw_pixel(x0 + x, y0 + y, color);
        }
    }
}

void gfx_draw_sprite_overwrite(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) / 8;
    data = sprite + 2 + frame * (w * pages);

    gfx_draw_bitmap(x, y, data, w, h);
}

void gfx_draw_sprite_self_masked(int x, int y, const unsigned char* sprite, int frame) {
    gfx_draw_sprite_overwrite(x, y, sprite, frame);
}

void gfx_draw_sprite_erase(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, sx, sy, pages;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) / 8;
    data = sprite + 2 + frame * (w * pages);

    for (sx = 0; sx < w; sx++) {
        for (sy = 0; sy < h; sy++) {
            int page = sy / 8;
            int bit = sy & 7;
            int idx = sx + page * w;
            if ((data[idx] >> bit) & 1) {
                gfx_draw_pixel(x + sx, y + sy, 0);
            }
        }
    }
}

void gfx_draw_sprite_plus_mask(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, sx, sy, pages;
    const unsigned char* data;

    if (!sprite) return;

    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) / 8;
    data = sprite + 2 + frame * (w * pages * 2);

    for (sx = 0; sx < w; sx++) {
        for (sy = 0; sy < h; sy++) {
            int page = sy / 8;
            int bit = sy & 7;
            int idx = (sx + page * w) * 2;
            int image_bit = (data[idx] >> bit) & 1;
            int mask_bit = (data[idx + 1] >> bit) & 1;

            if (mask_bit) {
                gfx_draw_pixel(x + sx, y + sy, image_bit);
            }
        }
    }
}

void gfx_draw_sprite_external_mask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame) {
    int w, h, sx, sy, pages;
    const unsigned char* image_data;
    const unsigned char* mask_data;

    if (!sprite || !mask) return;

    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) / 8;
    image_data = sprite + 2 + frame * (w * pages);
    mask_data = mask + 2 + mask_frame * (w * pages);

    for (sx = 0; sx < w; sx++) {
        for (sy = 0; sy < h; sy++) {
            int page = sy / 8;
            int bit = sy & 7;
            int idx = sx + page * w;
            int image_bit = (image_data[idx] >> bit) & 1;
            int mask_bit = (mask_data[idx] >> bit) & 1;

            if (mask_bit) {
                gfx_draw_pixel(x + sx, y + sy, image_bit);
            }
        }
    }
}

void gfx_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void gfx_set_text_size(int size) {
    if (size < 1) size = 1;
    if (size > 4) size = 4;
    text_size = size;
}

void gfx_set_text_wrap(bool w) {
    text_wrap = w;
}

bool gfx_get_text_wrap(void) {
    return text_wrap;
}

void gfx_set_text_raw(bool r) {
    text_raw = r;
}

bool gfx_get_text_raw(void) {
    return text_raw;
}

void gfx_set_text_color(int c) {
    text_color = c ? 1 : 0;
}

void gfx_set_text_bg(int c) {
    text_bg = c ? 1 : 0;
}

void gfx_set_invert(bool enable) {
    if (invert_enabled != enable) {
        invert_enabled = enable;
        memset(dirty_rows, 1, sizeof(dirty_rows));
        full_redraw_needed = true;
    }
}

void gfx_set_rgb_led(u8 r, u8 g, u8 b) {
    led_r = r;
    led_g = g;
    led_b = b;
}

void gfx_write_char(char c) {
    int glyph;
    int col, row;
    int cell_w = 6 * text_size;
    int cell_h = 8 * text_size;

    if (!text_raw) {
        if (c == '\n') {
            cursor_x = 0;
            cursor_y += cell_h;
            return;
        }
        if (c == '\r') {
            return;
        }
    }

    if (text_wrap && cursor_x + cell_w > ARDU_W) {
        cursor_x = 0;
        cursor_y += cell_h;
    }

    if ((unsigned char)c < 32 || (unsigned char)c > 127) {
        glyph = 95;
    } else {
        glyph = (unsigned char)c - 32;
    }

    for (col = 0; col < 5; col++) {
        unsigned char bits = font5x7[glyph][col];
        for (row = 0; row < 7; row++) {
            int px, py;
            int on = (bits >> row) & 1;

            for (py = 0; py < text_size; py++) {
                for (px = 0; px < text_size; px++) {
                    gfx_draw_pixel(
                        cursor_x + col * text_size + px,
                        cursor_y + row * text_size + py,
                        on ? text_color : text_bg
                    );
                }
            }
        }
    }

    for (row = 0; row < cell_h; row++) {
        int py = cursor_y + row;
        int px;
        if (py < 0 || py >= ARDU_H) continue;
        for (px = 0; px < text_size; px++) {
            int x = cursor_x + 5 * text_size + px;
            if (x >= 0 && x < ARDU_W) {
                backbuffer[py * ARDU_W + x] = mono_to_color(text_bg);
                mark_row_dirty(py);
            }
        }
    }

    for (col = 0; col < cell_w; col++) {
        int x = cursor_x + col;
        int py;
        if (x < 0 || x >= ARDU_W) continue;
        for (py = 0; py < text_size; py++) {
            int y = cursor_y + 7 * text_size + py;
            if (y >= 0 && y < ARDU_H) {
                backbuffer[y * ARDU_W + x] = mono_to_color(text_bg);
                mark_row_dirty(y);
            }
        }
    }

    cursor_x += cell_w;
}

void gfx_write_string(const char* s) {
    if (!s) return;
    while (*s) {
        gfx_write_char(*s++);
    }
}

void gfx_write_int(int v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    gfx_write_string(buf);
}

u32 gfx_get_last_present_ticks(void) {
    return 0;
}

u32 gfx_get_max_present_ticks(void) {
    return 0;
}

void gfx_reset_perf_counters(void) {
}
