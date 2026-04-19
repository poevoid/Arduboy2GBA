#include "graphics.h"
#include "background.h"

#include <gba_video.h>
#include <gba_dma.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define GBA_WIDTH 240
#define GBA_HEIGHT 160
#define ARDU_W 128
#define ARDU_H 64
#define ARDU_PAGES (ARDU_H / 8)

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0x7FFF

#define LED_INDICATOR_SIZE 16
#define LED_INDICATOR_RADIUS 8
#define LED_INDICATOR_SPACING 6
#define LED_OFFSET_X -4
#define LED_OFFSET_Y 1

static volatile u16* vram = (volatile u16*)0x6000000;

/* Native Arduboy-style packed framebuffer: 128 x 64 / 8 = 1024 bytes */
static unsigned char mono_buffer[ARDU_W * ARDU_PAGES] __attribute__((section(".ewram"), aligned(4)));
static unsigned char dirty_pages[ARDU_PAGES] __attribute__((section(".ewram"), aligned(4)));
static u16 row_expand[ARDU_W] __attribute__((section(".ewram"), aligned(4)));

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

static inline void mark_page_dirty(int page) {
    if (page >= 0 && page < ARDU_PAGES) {
        dirty_pages[page] = 1;
    }
}

static inline void mark_y_dirty(int y) {
    if (y >= 0 && y < ARDU_H) {
        dirty_pages[y >> 3] = 1;
    }
}

static inline void mark_y_span_dirty(int y0, int y1) {
    int p0;
    int p1;
    int p;

    if (y1 < 0 || y0 >= ARDU_H) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= ARDU_H) y1 = ARDU_H - 1;

    p0 = y0 >> 3;
    p1 = y1 >> 3;
    for (p = p0; p <= p1; ++p) {
        dirty_pages[p] = 1;
    }
}

static inline void mono_set_pixel_raw(int x, int y, int value) {
    unsigned char* cell;
    unsigned char mask;

    if (x < 0 || y < 0 || x >= ARDU_W || y >= ARDU_H) {
        return;
    }

    cell = &mono_buffer[(y >> 3) * ARDU_W + x];
    mask = (unsigned char)(1u << (y & 7));

    if (value) {
        *cell |= mask;
    } else {
        *cell &= (unsigned char)~mask;
    }

    mark_y_dirty(y);
}

static void copy_background_to_vram(void) {
    REG_DMA3CNT = 0;
    REG_DMA3SAD = (u32)gba_background;
    REG_DMA3DAD = (u32)vram;
    REG_DMA3CNT = DMA_ENABLE | DMA16 | (GBA_WIDTH * GBA_HEIGHT);
    REG_DMA3CNT = 0;
}

static void draw_led_indicator(void) {
    int left = ox - LED_INDICATOR_SPACING - LED_INDICATOR_SIZE + LED_OFFSET_X;
    int top = oy + (ARDU_H - LED_INDICATOR_SIZE) / 2 + LED_OFFSET_Y;
    int cx = left + LED_INDICATOR_RADIUS;
    int cy = top + LED_INDICATOR_RADIUS;
    int y;
    bool led_is_off = (led_r == 0 && led_g == 0 && led_b == 0);

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
                if (!led_is_off) {
                    vram[py * GBA_WIDTH + px] = fill;
                }
            } else if (dist2 <= LED_INDICATOR_RADIUS * LED_INDICATOR_RADIUS) {
                vram[py * GBA_WIDTH + px] = border;
            }
        }
    }
}

static void expand_page_row_to_scanline(int page, int row_in_page) {
    int x;
    unsigned char mask = (unsigned char)(1u << row_in_page);
    const unsigned char* src = &mono_buffer[page * ARDU_W];

    for (x = 0; x < ARDU_W; ++x) {
        row_expand[x] = mono_to_color((src[x] & mask) != 0);
    }
}

static void blit_bytes_overwrite(int x, int y, const unsigned char* src, int w, int h) {
    int src_pages;
    int dst_page;
    int yshift;
    int sx;

    if (!src || w <= 0 || h <= 0) return;
    if (x <= -w || x >= ARDU_W || y <= -h || y >= ARDU_H) return;

    src_pages = (h + 7) >> 3;
    dst_page = y >> 3;
    yshift = y & 7;

    for (sx = 0; sx < w; ++sx) {
        int dx = x + sx;
        int sp;

        if (dx < 0 || dx >= ARDU_W) continue;

        for (sp = 0; sp < src_pages; ++sp) {
            unsigned char s = src[sx + sp * w];
            int dp = dst_page + sp;

            if (yshift == 0) {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] = s;
                }
            } else {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] =
                        (unsigned char)((mono_buffer[dp * ARDU_W + dx] & ((1u << yshift) - 1u)) | (s << yshift));
                }
                if (dp + 1 >= 0 && dp + 1 < ARDU_PAGES) {
                    mono_buffer[(dp + 1) * ARDU_W + dx] =
                        (unsigned char)((mono_buffer[(dp + 1) * ARDU_W + dx] & (0xFFu << yshift)) | (s >> (8 - yshift)));
                }
            }
        }
    }

    mark_y_span_dirty(y, y + h - 1);
}

static void blit_bytes_self_masked(int x, int y, const unsigned char* src, int w, int h) {
    int src_pages;
    int dst_page;
    int yshift;
    int sx;

    if (!src || w <= 0 || h <= 0) return;
    if (x <= -w || x >= ARDU_W || y <= -h || y >= ARDU_H) return;

    src_pages = (h + 7) >> 3;
    dst_page = y >> 3;
    yshift = y & 7;

    for (sx = 0; sx < w; ++sx) {
        int dx = x + sx;
        int sp;

        if (dx < 0 || dx >= ARDU_W) continue;

        for (sp = 0; sp < src_pages; ++sp) {
            unsigned char s = src[sx + sp * w];
            int dp = dst_page + sp;

            if (s == 0) continue;

            if (yshift == 0) {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] |= s;
                }
            } else {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] |= (unsigned char)(s << yshift);
                }
                if (dp + 1 >= 0 && dp + 1 < ARDU_PAGES) {
                    mono_buffer[(dp + 1) * ARDU_W + dx] |= (unsigned char)(s >> (8 - yshift));
                }
            }
        }
    }

    mark_y_span_dirty(y, y + h - 1);
}

static void blit_bytes_erase(int x, int y, const unsigned char* src, int w, int h) {
    int src_pages;
    int dst_page;
    int yshift;
    int sx;

    if (!src || w <= 0 || h <= 0) return;
    if (x <= -w || x >= ARDU_W || y <= -h || y >= ARDU_H) return;

    src_pages = (h + 7) >> 3;
    dst_page = y >> 3;
    yshift = y & 7;

    for (sx = 0; sx < w; ++sx) {
        int dx = x + sx;
        int sp;

        if (dx < 0 || dx >= ARDU_W) continue;

        for (sp = 0; sp < src_pages; ++sp) {
            unsigned char s = src[sx + sp * w];
            int dp = dst_page + sp;

            if (s == 0) continue;

            if (yshift == 0) {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] &= (unsigned char)~s;
                }
            } else {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    mono_buffer[dp * ARDU_W + dx] &= (unsigned char)~(s << yshift);
                }
                if (dp + 1 >= 0 && dp + 1 < ARDU_PAGES) {
                    mono_buffer[(dp + 1) * ARDU_W + dx] &= (unsigned char)~(s >> (8 - yshift));
                }
            }
        }
    }

    mark_y_span_dirty(y, y + h - 1);
}

static void blit_bytes_plus_mask(int x, int y, const unsigned char* src, int w, int h) {
    int src_pages;
    int dst_page;
    int yshift;
    int sx;

    if (!src || w <= 0 || h <= 0) return;
    if (x <= -w || x >= ARDU_W || y <= -h || y >= ARDU_H) return;

    src_pages = (h + 7) >> 3;
    dst_page = y >> 3;
    yshift = y & 7;

    for (sx = 0; sx < w; ++sx) {
        int dx = x + sx;
        int sp;

        if (dx < 0 || dx >= ARDU_W) continue;

        for (sp = 0; sp < src_pages; ++sp) {
            int idx = (sx + sp * w) * 2;
            unsigned char image = src[idx];
            unsigned char mask = src[idx + 1];
            int dp = dst_page + sp;

            if (mask == 0) continue;

            if (yshift == 0) {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    unsigned char* d = &mono_buffer[dp * ARDU_W + dx];
                    *d = (unsigned char)((*d & (unsigned char)~mask) | (image & mask));
                }
            } else {
                unsigned char image_lo = (unsigned char)(image << yshift);
                unsigned char mask_lo = (unsigned char)(mask << yshift);
                unsigned char image_hi = (unsigned char)(image >> (8 - yshift));
                unsigned char mask_hi = (unsigned char)(mask >> (8 - yshift));

                if (dp >= 0 && dp < ARDU_PAGES) {
                    unsigned char* d0 = &mono_buffer[dp * ARDU_W + dx];
                    *d0 = (unsigned char)((*d0 & (unsigned char)~mask_lo) | (image_lo & mask_lo));
                }
                if (dp + 1 >= 0 && dp + 1 < ARDU_PAGES) {
                    unsigned char* d1 = &mono_buffer[(dp + 1) * ARDU_W + dx];
                    *d1 = (unsigned char)((*d1 & (unsigned char)~mask_hi) | (image_hi & mask_hi));
                }
            }
        }
    }

    mark_y_span_dirty(y, y + h - 1);
}

static void blit_bytes_external_mask(int x, int y, const unsigned char* image_src, const unsigned char* mask_src, int w, int h) {
    int src_pages;
    int dst_page;
    int yshift;
    int sx;

    if (!image_src || !mask_src || w <= 0 || h <= 0) return;
    if (x <= -w || x >= ARDU_W || y <= -h || y >= ARDU_H) return;

    src_pages = (h + 7) >> 3;
    dst_page = y >> 3;
    yshift = y & 7;

    for (sx = 0; sx < w; ++sx) {
        int dx = x + sx;
        int sp;

        if (dx < 0 || dx >= ARDU_W) continue;

        for (sp = 0; sp < src_pages; ++sp) {
            unsigned char image = image_src[sx + sp * w];
            unsigned char mask = mask_src[sx + sp * w];
            int dp = dst_page + sp;

            if (mask == 0) continue;

            if (yshift == 0) {
                if (dp >= 0 && dp < ARDU_PAGES) {
                    unsigned char* d = &mono_buffer[dp * ARDU_W + dx];
                    *d = (unsigned char)((*d & (unsigned char)~mask) | (image & mask));
                }
            } else {
                unsigned char image_lo = (unsigned char)(image << yshift);
                unsigned char mask_lo = (unsigned char)(mask << yshift);
                unsigned char image_hi = (unsigned char)(image >> (8 - yshift));
                unsigned char mask_hi = (unsigned char)(mask >> (8 - yshift));

                if (dp >= 0 && dp < ARDU_PAGES) {
                    unsigned char* d0 = &mono_buffer[dp * ARDU_W + dx];
                    *d0 = (unsigned char)((*d0 & (unsigned char)~mask_lo) | (image_lo & mask_lo));
                }
                if (dp + 1 >= 0 && dp + 1 < ARDU_PAGES) {
                    unsigned char* d1 = &mono_buffer[(dp + 1) * ARDU_W + dx];
                    *d1 = (unsigned char)((*d1 & (unsigned char)~mask_hi) | (image_hi & mask_hi));
                }
            }
        }
    }

    mark_y_span_dirty(y, y + h - 1);
}

void gfx_init(void) {
    REG_DISPCNT = MODE_3 | BG2_ENABLE;
    copy_background_to_vram();
    gfx_clear();
}

void gfx_clear(void) {
    memset(mono_buffer, 0, sizeof(mono_buffer));
    memset(dirty_pages, 1, sizeof(dirty_pages));
    full_redraw_needed = true;
    cursor_x = 0;
    cursor_y = 0;
}

void gfx_present(void) {
    int page;
    for (page = 0; page < ARDU_PAGES; ++page) {
        if (full_redraw_needed || dirty_pages[page]) {
            int row;
            for (row = 0; row < 8; ++row) {
                int screen_y = page * 8 + row;
                volatile u16* dst;

                expand_page_row_to_scanline(page, row);
                dst = &vram[(oy + screen_y) * GBA_WIDTH + ox];
                dma_copy_row_16(row_expand, dst, ARDU_W);
            }
            dirty_pages[page] = 0;
        }
    }

    REG_DMA3CNT = 0;
    full_redraw_needed = false;
    draw_led_indicator();
}

void gfx_draw_pixel(int x, int y, u16 color) {
    mono_set_pixel_raw(x, y, color ? 1 : 0);
}

void gfx_draw_fast_hline(int x, int y, int w, u16 color) {
    int i;
    int bit;

    if (y < 0 || y >= ARDU_H || w <= 0) return;
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (x + w > ARDU_W) {
        w = ARDU_W - x;
    }
    if (w <= 0) return;

    bit = color ? 1 : 0;
    for (i = 0; i < w; ++i) {
        mono_set_pixel_raw(x + i, y, bit);
    }
}

void gfx_draw_fast_vline(int x, int y, int h, u16 color) {
    int i;
    int bit;

    if (x < 0 || x >= ARDU_W || h <= 0) return;
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (y + h > ARDU_H) {
        h = ARDU_H - y;
    }
    if (h <= 0) return;

    bit = color ? 1 : 0;
    for (i = 0; i < h; ++i) {
        mono_set_pixel_raw(x, y + i, bit);
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
    for (j = 0; j < h; ++j) {
        gfx_draw_fast_hline(x, y + j, w, color);
    }
}

void gfx_fill_screen(u16 color) {
    memset(mono_buffer, color ? 0xFF : 0x00, sizeof(mono_buffer));
    memset(dirty_pages, 1, sizeof(dirty_pages));
    full_redraw_needed = true;
}

void gfx_draw_bitmap(int x, int y, const unsigned char* bmp, int w, int h) {
    blit_bytes_overwrite(x, y, bmp, w, h);
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

    for (y = -r; y <= r; ++y) {
        int x;
        int span = 0;

        while ((span + 1) * (span + 1) + y * y <= r * r) {
            span++;
        }

        for (x = -span + 1; x <= span - 1; ++x) {
            gfx_draw_pixel(x0 + x, y0 + y, color);
        }
    }
}

void gfx_draw_round_rect(int x, int y, int w, int h, int r, u16 color) {
    int dx, dy;

    if (w <= 0 || h <= 0) return;

    if (r < 0) r = 0;
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    gfx_draw_fast_hline(x + r, y, w - 2 * r, color);
    gfx_draw_fast_hline(x + r, y + h - 1, w - 2 * r, color);
    gfx_draw_fast_vline(x, y + r, h - 2 * r, color);
    gfx_draw_fast_vline(x + w - 1, y + r, h - 2 * r, color);

    for (dy = -r; dy <= r; ++dy) {
        for (dx = -r; dx <= r; ++dx) {
            if (dx * dx + dy * dy <= r * r) {
                if (dx <= 0 && dy <= 0) gfx_draw_pixel(x + r + dx, y + r + dy, color);
                if (dx >= 0 && dy <= 0) gfx_draw_pixel(x + w - r - 1 + dx, y + r + dy, color);
                if (dx <= 0 && dy >= 0) gfx_draw_pixel(x + r + dx, y + h - r - 1 + dy, color);
                if (dx >= 0 && dy >= 0) gfx_draw_pixel(x + w - r - 1 + dx, y + h - r - 1 + dy, color);
            }
        }
    }
}

void gfx_draw_sprite_overwrite(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages;
    const unsigned char* data;

    if (!sprite) return;
    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) >> 3;
    data = sprite + 2 + frame * (w * pages);
    blit_bytes_overwrite(x, y, data, w, h);
}

void gfx_draw_sprite_self_masked(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages;
    const unsigned char* data;

    if (!sprite) return;
    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) >> 3;
    data = sprite + 2 + frame * (w * pages);
    blit_bytes_self_masked(x, y, data, w, h);
}

void gfx_draw_sprite_erase(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages;
    const unsigned char* data;

    if (!sprite) return;
    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) >> 3;
    data = sprite + 2 + frame * (w * pages);
    blit_bytes_erase(x, y, data, w, h);
}

void gfx_draw_sprite_plus_mask(int x, int y, const unsigned char* sprite, int frame) {
    int w, h, pages;
    const unsigned char* data;

    if (!sprite) return;
    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) >> 3;
    data = sprite + 2 + frame * (w * pages * 2);
    blit_bytes_plus_mask(x, y, data, w, h);
}

void gfx_draw_sprite_external_mask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame) {
    int w, h, pages;
    const unsigned char* image_data;
    const unsigned char* mask_data;

    if (!sprite || !mask) return;

    w = sprite[0];
    h = sprite[1];
    pages = (h + 7) >> 3;
    image_data = sprite + 2 + frame * (w * pages);
    mask_data = mask + 2 + mask_frame * (w * pages);

    blit_bytes_external_mask(x, y, image_data, mask_data, w, h);
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
        memset(dirty_pages, 1, sizeof(dirty_pages));
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

    if (text_size == 1) {
        for (col = 0; col < 5; ++col) {
            unsigned char bits = font5x7[glyph][col];
            for (row = 0; row < 7; ++row) {
                mono_set_pixel_raw(cursor_x + col, cursor_y + row, ((bits >> row) & 1) ? text_color : text_bg);
            }
        }

        for (row = 0; row < 8; ++row) {
            mono_set_pixel_raw(cursor_x + 5, cursor_y + row, text_bg);
        }
        for (col = 0; col < 6; ++col) {
            mono_set_pixel_raw(cursor_x + col, cursor_y + 7, text_bg);
        }

        cursor_x += cell_w;
        return;
    }

    for (col = 0; col < 5; ++col) {
        unsigned char bits = font5x7[glyph][col];
        for (row = 0; row < 7; ++row) {
            int bit = ((bits >> row) & 1) ? text_color : text_bg;
            int px, py;
            for (py = 0; py < text_size; ++py) {
                for (px = 0; px < text_size; ++px) {
                    mono_set_pixel_raw(
                        cursor_x + col * text_size + px,
                        cursor_y + row * text_size + py,
                        bit
                    );
                }
            }
        }
    }

    for (row = 0; row < cell_h; ++row) {
        int px;
        for (px = 0; px < text_size; ++px) {
            mono_set_pixel_raw(cursor_x + 5 * text_size + px, cursor_y + row, text_bg);
        }
    }

    for (col = 0; col < cell_w; ++col) {
        int py;
        for (py = 0; py < text_size; ++py) {
            mono_set_pixel_raw(cursor_x + col, cursor_y + 7 * text_size + py, text_bg);
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
