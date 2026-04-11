#ifndef ARDUBOY_COMPAT_H
#define ARDUBOY_COMPAT_H

#include <gba_types.h>
#include <gba_input.h>

#define BLACK 0x0000
#define WHITE 0x7FFF

#define UP_BUTTON    KEY_UP
#define DOWN_BUTTON  KEY_DOWN
#define LEFT_BUTTON  KEY_LEFT
#define RIGHT_BUTTON KEY_RIGHT
#define A_BUTTON     KEY_A
#define B_BUTTON     KEY_B

void ab_init();
void ab_clear();
void ab_display();

void ab_drawPixel(int x, int y, u16 color);
void ab_drawFastHLine(int x, int y, int w, u16 color);
void ab_drawFastVLine(int x, int y, int h, u16 color);

void ab_drawRect(int x, int y, int w, int h, u16 color);
void ab_fillRect(int x, int y, int w, int h, u16 color);
void ab_fillScreen(u16 color);

void ab_drawBitmap(int x, int y, const unsigned char* bmp, int w, int h);

void ab_drawOverwrite(int x, int y, const unsigned char* sprite, int frame);
void ab_drawSelfMasked(int x, int y, const unsigned char* sprite, int frame);
void ab_drawErase(int x, int y, const unsigned char* sprite, int frame);
void ab_drawPlusMask(int x, int y, const unsigned char* sprite, int frame);
void ab_drawExternalMask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame, int mask_frame);

void ab_setCursor(int x, int y);
void ab_print(const char* str);

u16 ab_pollButtons();
int ab_pressed(u16 key);

void ab_tone(int freq, int duration);
void ab_playScore(const unsigned char* score);
void ab_stopScore();

#endif
