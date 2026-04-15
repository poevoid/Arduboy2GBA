#ifndef ARDUBOY_COMPAT_H
#define ARDUBOY_COMPAT_H

#include <gba_types.h>
#include <gba_input.h>
#include <stdbool.h>

#define WIDTH 128
#define HEIGHT 64

#define WHITE 0x7FFF
#define BLACK 0x0000

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef F
#define F(x) (x)
#endif

#define A_BUTTON     KEY_A
#define B_BUTTON     KEY_B
#define UP_BUTTON    KEY_UP
#define DOWN_BUTTON  KEY_DOWN
#define LEFT_BUTTON  KEY_LEFT
#define RIGHT_BUTTON KEY_RIGHT

#define AB_CHAR_WIDTH 5
#define AB_CHAR_HEIGHT 7
#define AB_CHAR_SPACING 1
#define AB_LINE_SPACING 1

#define AB_PRINT(x) ab_print(x)

void ab_begin(void);
void ab_beginNoLogo(void);
void ab_initRandomSeed(void);

void ab_clear(void);
void ab_display(void);

void ab_drawPixel(int x, int y, int c);
void ab_drawFastHLine(int x, int y, int w, int c);
void ab_drawFastVLine(int x, int y, int h, int c);
void ab_drawRect(int x, int y, int w, int h, int c);
void ab_fillRect(int x, int y, int w, int h, int c);

void ab_setCursor(int x, int y);
void ab_setTextSize(int s);
void ab_setTextWrap(bool w);
bool ab_getTextWrap(void);
void ab_setTextRawMode(bool r);
bool ab_getTextRawMode(void);
void ab_setTextColor(int c);
void ab_setTextBackground(int c);

void ab_print(const char* s);
void ab_print(int v);
void ab_print(char c);
void ab_print(float v);

void ab_pollButtons(void);
bool ab_pressed(u16 key);
bool ab_justPressed(u16 key);

void ab_setFrameRate(int fps);
bool ab_nextFrame(void);
void ab_delay(int ms);
void ab_idle(void);

int ab_random(int min, int max);
void ab_tone(int freq, int duration);
void ab_playScore(const unsigned char* score);
void ab_stopScore(void);

/* Time scaling */
void ab_setTimeScale(float scale);
float ab_getTimeScale(void);

/* Debug/telemetry helpers */
int ab_get_frame_duration_ms(void);
int ab_get_dropped_frames(void);
int ab_get_last_present_ticks(void);
int ab_get_max_present_ticks(void);

#endif
