#ifndef ARDUBOY_COMPAT_H
#define ARDUBOY_COMPAT_H

#include <gba_types.h>
#include <gba_input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

#ifndef pgm_read_ptr
#define pgm_read_ptr(addr) ((uintptr_t)(*(const void * const *)(addr)))
#endif

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif

template <typename T>
static inline T min(T a, T b) {
    return (a < b) ? a : b;
}

template <typename T>
static inline T max(T a, T b) {
    return (a > b) ? a : b;
}

static inline char* ltoa(long value, char* str, int base) {
    if (base == 16) {
        snprintf(str, 34, "%lX", value);
    } else {
        snprintf(str, 34, "%ld", value);
    }
    return str;
}

#define A_BUTTON     KEY_A
#define B_BUTTON     KEY_B
#define UP_BUTTON    KEY_UP
#define DOWN_BUTTON  KEY_DOWN
#define LEFT_BUTTON  KEY_LEFT
#define RIGHT_BUTTON KEY_RIGHT

#define PIN_SPEAKER_1 1
#define PIN_SPEAKER_2 2

#define RED_LED   1
#define GREEN_LED 2
#define BLUE_LED  4

#define RGB_ON  1
#define RGB_OFF 0

#define CLEAR_BUFFER 1
#define HEX 16

#define EEPROM_STORAGE_SPACE_START 0

#define AB_CHAR_WIDTH 5
#define AB_CHAR_HEIGHT 7
#define AB_CHAR_SPACING 1
#define AB_LINE_SPACING 1

#define AB_PRINT(x) ab_print(x)
#define AB_PRINTLN(x) ab_println(x)

void ab_begin(void);
void ab_beginNoLogo(void);
void ab_initRandomSeed(void);
void ab_bootLogoSpritesSelfMasked(void);

void ab_clear(void);
void ab_display(void);
void ab_display(int clear_buffer);
void ab_invert(bool enable);

void ab_drawPixel(int x, int y, int c = WHITE);
void ab_drawFastHLine(int x, int y, int w, int c = WHITE);
void ab_drawFastVLine(int x, int y, int h, int c = WHITE);
void ab_drawRect(int x, int y, int w, int h, int c = WHITE);
void ab_fillRect(int x, int y, int w, int h, int c = WHITE);
void ab_fillScreen(int c = BLACK);
void ab_drawCircle(int x, int y, int r, int c = WHITE);
void ab_fillCircle(int x, int y, int r, int c = WHITE);
void ab_drawLine(int x0, int y0, int x1, int y1, int c = WHITE);
void ab_drawRoundRect(int x, int y, int w, int h, int r, int c = WHITE);
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
void ab_print(unsigned int v);
void ab_print(char c);
void ab_print(float v);
void ab_print(int v, int base);
void ab_print(unsigned int v, int base);
void ab_print(unsigned char v, int base);

void ab_println(void);
void ab_println(const char* s);
void ab_println(int v);
void ab_println(char c);
void ab_println(float v);

void ab_pollButtons(void);
bool ab_pressed(u16 key);
bool ab_justPressed(u16 key);
bool ab_notPressed(u16 key);

void ab_setFrameRate(int fps);
bool ab_nextFrame(void);
bool ab_everyXFrames(int frames);
void ab_delay(int ms);
void ab_idle(void);

int ab_random(int max);
int ab_random(int min, int max);
float ab_radians(float deg);
void ab_tone(int freq, int duration = 0);
void ab_noTone(void);
void ab_playScore(const unsigned char* score);
void ab_stopScore(void);

bool ab_audio_enabled(void);
void ab_audio_on(void);
void ab_audio_off(void);
bool ab_score_playing(void);
void ab_set_tone_mutes_score(bool mute);

void ab_setRGBled(u8 red, u8 green, u8 blue);
void ab_setRGBled(u8 color, u8 value);
void ab_digitalWriteRGB(u8 color, u8 value);
void ab_digitalWriteRGB(u8 red, u8 green, u8 blue);
void ab_freeRGBled(void);

/* Time scaling */
void ab_setTimeScale(float scale);
float ab_getTimeScale(void);

/* Debug/telemetry helpers */
int ab_get_frame_duration_ms(void);
int ab_get_dropped_frames(void);
int ab_get_last_present_ticks(void);
int ab_get_max_present_ticks(void);

class EEPROMClass {
public:
    unsigned char read(int address) const;
    void write(int address, unsigned char value);
    void update(int address, unsigned char value);

    template <typename T>
    void get(int address, T& value) const {
        unsigned char* dst = reinterpret_cast<unsigned char*>(&value);
        for (size_t i = 0; i < sizeof(T); ++i) {
            dst[i] = read(address + (int)i);
        }
    }

    template <typename T>
    void put(int address, const T& value) {
        const unsigned char* src = reinterpret_cast<const unsigned char*>(&value);
        for (size_t i = 0; i < sizeof(T); ++i) {
            update(address + (int)i, src[i]);
        }
    }
};

extern EEPROMClass EEPROM;

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

    void begin() {}
    void on() { ab_audio_on(); }
    void off() { ab_audio_off(); }
    void saveOnOff() {}
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

class ArduboyTones {
public:
    ArduboyTones() {}
    template <typename T>
    explicit ArduboyTones(T) {}

    void tone(unsigned int frequency, unsigned long duration = 0) {
        if (ab_audio_enabled()) {
            ab_tone((int)frequency, (int)duration);
        }
    }
};

class Arduboy2Base {
public:
    ArduboyAudioShim audio;

    void begin() { ab_begin(); }
    void beginNoLogo() { ab_beginNoLogo(); }
    void boot() { ab_begin(); }
    void bootLogoSpritesSelfMasked() { ab_bootLogoSpritesSelfMasked(); }
    void initRandomSeed() { ab_initRandomSeed(); }

    void clear() { ab_clear(); }
    void display() { ab_display(); }
    void display(int clear_buffer) { ab_display(clear_buffer); }
    void invert(bool enable) { ab_invert(enable); }

    void drawPixel(int x, int y, int c = WHITE) { ab_drawPixel(x, y, c); }
    void drawFastHLine(int x, int y, int w, int c = WHITE) { ab_drawFastHLine(x, y, w, c); }
    void drawFastVLine(int x, int y, int h, int c = WHITE) { ab_drawFastVLine(x, y, h, c); }
    void drawRect(int x, int y, int w, int h, int c = WHITE) { ab_drawRect(x, y, w, h, c); }
    void fillRect(int x, int y, int w, int h, int c = WHITE) { ab_fillRect(x, y, w, h, c); }
    void fillScreen(int c = BLACK) { ab_fillScreen(c); }
    void drawCircle(int x, int y, int r, int c = WHITE) { ab_drawCircle(x, y, r, c); }
    void fillCircle(int x, int y, int r, int c = WHITE) { ab_fillCircle(x, y, r, c); }
    void drawLine(int x0, int y0, int x1, int y1, int c = WHITE) { ab_drawLine(x0, y0, x1, y1, c); }
    void drawRoundRect(int x, int y, int w, int h, int r, int c = WHITE) { ab_drawRoundRect(x, y, w, h, r, c); }
    void drawBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c = WHITE) { ab_drawBitmap(x, y, bmp, w, h, c); }
    void drawSlowXYBitmap(int x, int y, const unsigned char* bmp, int w, int h, int c = WHITE) { ab_drawBitmap(x, y, bmp, w, h, c); }

    void setCursor(int x, int y) { ab_setCursor(x, y); }
    void setTextSize(int s) { ab_setTextSize(s); }
    void setTextWrap(bool w) { ab_setTextWrap(w); }
    bool getTextWrap() const { return ab_getTextWrap(); }
    void setTextRawMode(bool r) { ab_setTextRawMode(r); }
    bool getTextRawMode() const { return ab_getTextRawMode(); }
    void setTextColor(int c) { ab_setTextColor(c); }
    void setTextBackground(int c) { ab_setTextBackground(c); }

    static constexpr uint8_t getCharacterWidth() { return AB_CHAR_WIDTH; }
    static constexpr uint8_t getCharacterSpacing() { return AB_CHAR_SPACING; }
    static constexpr uint8_t getCharacterHeight() { return AB_CHAR_HEIGHT; }
    static constexpr uint8_t getLineSpacing() { return AB_LINE_SPACING; }

    void print(const char* s) { ab_print(s); }
    void print(int v) { ab_print(v); }
    void print(unsigned int v) { ab_print(v); }
    void print(char c) { ab_print(c); }
    void print(float v) { ab_print(v); }
    void print(int v, int base) { ab_print(v, base); }
    void print(unsigned int v, int base) { ab_print(v, base); }
    void print(unsigned char v, int base) { ab_print(v, base); }

    void println() { ab_println(); }
    void println(const char* s) { ab_println(s); }
    void println(int v) { ab_println(v); }
    void println(char c) { ab_println(c); }
    void println(float v) { ab_println(v); }

    void pollButtons() { ab_pollButtons(); }
    bool pressed(u16 key) { return ab_pressed(key); }
    bool justPressed(u16 key) { return ab_justPressed(key); }
    bool notPressed(u16 key) { return ab_notPressed(key); }

    void setFrameRate(int fps) { ab_setFrameRate(fps); }
    bool nextFrame() { return ab_nextFrame(); }
    bool everyXFrames(int frames) { return ab_everyXFrames(frames); }
    void idle() { ab_idle(); }

    void delayShort(unsigned long ms) { ab_delay((int)ms); }

    void setRGBled(u8 red, u8 green, u8 blue) { ab_setRGBled(red, green, blue); }
    void setRGBled(u8 color, u8 value) { ab_setRGBled(color, value); }
    void digitalWriteRGB(u8 color, u8 value) { ab_digitalWriteRGB(color, value); }
    void digitalWriteRGB(u8 red, u8 green, u8 blue) { ab_digitalWriteRGB(red, green, blue); }
    void freeRGBled() { ab_freeRGBled(); }
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
    constexpr unsigned int freq(T hz) const {
        return (hz <= 0) ? 0u : (unsigned int)(hz + 0.5);
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

    void tone(unsigned int frequency, unsigned long duration = 0) {
        if (ab_audio_enabled()) {
            ab_tone((int)frequency, (int)duration);
        }
    }

    void toneMutesScore(bool mute) {
        ab_set_tone_mutes_score(mute);
    }
};

class ArduboyPlayTunes : public ArduboyPlaytune {
public:
    ArduboyPlayTunes() : ArduboyPlaytune() {}

    template <typename T>
    explicit ArduboyPlayTunes(T t) : ArduboyPlaytune(t) {}
};

#endif
