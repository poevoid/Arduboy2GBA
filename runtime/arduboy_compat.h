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

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif

#define A_BUTTON     KEY_A
#define B_BUTTON     KEY_B
#define UP_BUTTON    KEY_UP
#define DOWN_BUTTON  KEY_DOWN
#define LEFT_BUTTON  KEY_LEFT
#define RIGHT_BUTTON KEY_RIGHT

#define PIN_SPEAKER_1 1
#define PIN_SPEAKER_2 2

#define AB_CHAR_WIDTH 5
#define AB_CHAR_HEIGHT 7
#define AB_CHAR_SPACING 1
#define AB_LINE_SPACING 1

#define AB_PRINT(x) ab_print(x)
#define AB_PRINTLN(x) ab_println(x)

void ab_begin(void);
void ab_beginNoLogo(void);
void ab_initRandomSeed(void);

void ab_clear(void);
void ab_display(void);
void ab_invert(bool enable);

void ab_drawPixel(int x, int y, int c = WHITE);
void ab_drawFastHLine(int x, int y, int w, int c = WHITE);
void ab_drawFastVLine(int x, int y, int h, int c = WHITE);
void ab_drawRect(int x, int y, int w, int h, int c = WHITE);
void ab_fillRect(int x, int y, int w, int h, int c = WHITE);
void ab_fillScreen(int c = BLACK);
void ab_drawBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c = WHITE);

void ab_drawOverwrite(int x, int y, const unsigned char* sprite, int frame = 0);
void ab_drawSelfMasked(int x, int y, const unsigned char* sprite, int frame = 0);
void ab_drawErase(int x, int y, const unsigned char* sprite, int frame = 0);
void ab_drawPlusMask(int x, int y, const unsigned char* sprite, int frame = 0);
void ab_drawExternalMask(int x, int y, const unsigned char* sprite, const unsigned char* mask, int frame = 0, int mask_frame = 0);

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

void ab_println(void);
void ab_println(const char* s);
void ab_println(int v);
void ab_println(char c);
void ab_println(float v);

void ab_pollButtons(void);
bool ab_pressed(u16 key);
bool ab_justPressed(u16 key);

void ab_setFrameRate(int fps);
bool ab_nextFrame(void);
void ab_delay(int ms);
void ab_idle(void);

int ab_random(int min, int max);
void ab_tone(int freq, int duration = 0);
void ab_noTone(void);
void ab_playScore(const unsigned char* score);
void ab_stopScore(void);

bool ab_audio_enabled(void);
void ab_audio_on(void);
void ab_audio_off(void);
bool ab_score_playing(void);

/* Time scaling */
void ab_setTimeScale(float scale);
float ab_getTimeScale(void);

/* Debug/telemetry helpers */
int ab_get_frame_duration_ms(void);
int ab_get_dropped_frames(void);
int ab_get_last_present_ticks(void);
int ab_get_max_present_ticks(void);

class ArduboyAudioShim;

class ArduboyAudioEnabledProxy {
public:
    ArduboyAudioShim* owner;

    ArduboyAudioEnabledProxy() : owner(0) {}
    explicit ArduboyAudioEnabledProxy(ArduboyAudioShim* o) : owner(o) {}

    bool operator()() const;
    operator bool() const;
};

class ArduboyAudioShim {
public:
    ArduboyAudioEnabledProxy enabled;

    ArduboyAudioShim() : enabled(this) {}

    void on() { ab_audio_on(); }
    void off() { ab_audio_off(); }
    bool isEnabled() const { return ab_audio_enabled(); }
};

inline bool ArduboyAudioEnabledProxy::operator()() const {
    (void)owner;
    return ab_audio_enabled();
}

inline ArduboyAudioEnabledProxy::operator bool() const {
    (void)owner;
    return ab_audio_enabled();
}

class Arduboy2Base {
public:
    ArduboyAudioShim audio;

    void begin() { ab_begin(); }
    void beginNoLogo() { ab_beginNoLogo(); }
    void initRandomSeed() { ab_initRandomSeed(); }

    void clear() { ab_clear(); }
    void display() { ab_display(); }
    void invert(bool enable) { ab_invert(enable); }

    void drawPixel(int x, int y, int c = WHITE) { ab_drawPixel(x, y, c); }
    void drawFastHLine(int x, int y, int w, int c = WHITE) { ab_drawFastHLine(x, y, w, c); }
    void drawFastVLine(int x, int y, int h, int c = WHITE) { ab_drawFastVLine(x, y, h, c); }
    void drawRect(int x, int y, int w, int h, int c = WHITE) { ab_drawRect(x, y, w, h, c); }
    void fillRect(int x, int y, int w, int h, int c = WHITE) { ab_fillRect(x, y, w, h, c); }
    void fillScreen(int c = BLACK) { ab_fillScreen(c); }
    void drawBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c = WHITE) { ab_drawBitmap(x, y, bmp, w, h, c); }
    void drawSlowXYBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c = WHITE) { ab_drawBitmap(x, y, bmp, w, h, c); }

    void setCursor(int x, int y) { ab_setCursor(x, y); }
    void setTextSize(int s) { ab_setTextSize(s); }
    void setTextWrap(bool w) { ab_setTextWrap(w); }
    void setTextColor(int c) { ab_setTextColor(c); }
    void setTextBackground(int c) { ab_setTextBackground(c); }

    void print(const char* s) { ab_print(s); }
    void print(int v) { ab_print(v); }
    void print(char c) { ab_print(c); }
    void print(float v) { ab_print(v); }

    void println() { ab_println(); }
    void println(const char* s) { ab_println(s); }
    void println(int v) { ab_println(v); }
    void println(char c) { ab_println(c); }
    void println(float v) { ab_println(v); }

    void pollButtons() { ab_pollButtons(); }
    bool pressed(u16 key) { return ab_pressed(key); }
    bool justPressed(u16 key) { return ab_justPressed(key); }

    void setFrameRate(int fps) { ab_setFrameRate(fps); }
    bool nextFrame() { return ab_nextFrame(); }

    void delayShort(unsigned long ms) { ab_delay((int)ms); }
};

typedef Arduboy2Base Arduboy2;
typedef Arduboy2Base Arduboy;

extern Arduboy2Base arduboy;

class BeepPin1 {
public:
    int duration;

    BeepPin1() : duration(0) {}

    void begin() {}

    void timer() {
        if (duration > 0) {
            duration--;
        }
    }

    template <typename T>
    unsigned int freq(T hz) const {
        double value = (double)hz;
        if (value <= 0.0) {
            return 0;
        }
        return (unsigned int)(value + 0.5);
    }

    void tone(unsigned int freq_hz, unsigned long dur = 0) {
        duration = (dur == 0) ? -1 : (int)dur;
        ab_tone((int)freq_hz, (int)dur);
    }

    void noTone() {
        duration = 0;
        ab_noTone();
    }
};

class BeepPin2 : public BeepPin1 {
};

class ArduboyPlaytune {
public:
    bool (*audio_enabled_cb)(void);

    ArduboyPlaytune() : audio_enabled_cb(0) {}

    template <typename T>
    explicit ArduboyPlaytune(T) : audio_enabled_cb(0) {}

    void initChannel(unsigned char) {}

    void playScore(const unsigned char* score) {
        if (ab_audio_enabled()) {
            ab_playScore(score);
        }
    }

    void stopScore() {
        ab_stopScore();
    }

    bool playing() const {
        return ab_score_playing();
    }
};

class ArduboyPlayTunes : public ArduboyPlaytune {
public:
    ArduboyPlayTunes() : ArduboyPlaytune() {}

    template <typename T>
    explicit ArduboyPlayTunes(T t) : ArduboyPlaytune(t) {}
};

#endif
