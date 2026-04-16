#include "arduboy_compat.h"

#include <math.h>
#include <string.h>
/*
RGBled

This sketch demonstrates controlling the Arduboy's RGB LED,
in both analog and digital modes.
*/

/*
Written in 2018 by Scott Allen saydisp-git@yahoo.ca

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

// The frame rate determines the button auto-repeat rate
#define FRAME_RATE 25

// The increment/decrement amount when auto-repeating
#define REPEAT_AMOUNT 3

// Delay time before button auto-repeat starts, in milliseconds
#define REPEAT_DELAY 700

// Calculation of the number of frames to wait before button auto-repeat starts
#define DELAY_FRAMES (REPEAT_DELAY / (1000 / FRAME_RATE))

#define ANALOG false
#define DIGITAL true

#define ANALOG_MAX 255

// Color array index
enum class Color {
  RED,
  GREEN,
  BLUE,
  COUNT
};

// Map LED color index to LED name
const unsigned char LEDpin[(unsigned char)(Color::COUNT)] = {
  RED_LED,
  GREEN_LED,
  BLUE_LED
};

// Analog LED values
unsigned char analogValue[3] = { 0, 0, 0};
// Digital LED states
unsigned char digitalState[3] = { RGB_OFF, RGB_OFF, RGB_OFF };

unsigned char analogSelected = (unsigned char)(Color::RED);
unsigned char digitalSelected = (unsigned char)(Color::RED);

bool controlMode = ANALOG;

// Button repeat handling
unsigned int delayCount = 0;
bool repeating = false;

// ============================= SETUP ===================================
void setup();
void loop();
void modeAnalog();
void modeDigital();
void reset();
void valueInc(unsigned char amount);
void valueDec(unsigned char amount);
void analogSelectInc();
void analogSelectDec();
void digitalSelectInc();
void digitalSelectDec();
void selectInc(unsigned char &index);
void selectDec(unsigned char &index);
void analogSet();
void digitalSet();
void startButtonDelay();
void stopButtonRepeat();
void renderScreen();
void drawAnalog(int y, Color color, const char* name);
void drawBar(int y, Color color, unsigned char value);
void drawDigital(int y, Color color, const char* name);
void printValue(unsigned char val);

void setup() {
  ab_begin();
  ab_setFrameRate(FRAME_RATE);
  analogSet();
}
// =======================================================================


// =========================== MAIN LOOP =================================
void loop() {
  if (!ab_nextFrame()) {
    return;
  }

  ab_pollButtons();

  // Toggle analog/digital control mode
  if (ab_justPressed(A_BUTTON)) {
    if ((controlMode = !controlMode) == DIGITAL) {
      arduboy.freeRGBled();
      digitalSet();
    }
    else {
      analogSet();
    }
  }

  // Reset to Analog mode and all LEDs off
  if (ab_justPressed(B_BUTTON)) {
    reset();
  }

  // Handle D-pad buttons for current mode
  if (controlMode == ANALOG) {
    modeAnalog();
  }
  else {
    modeDigital();
  }

  // Handle delay before button auto-repeat starts
  if ((delayCount != 0) && (--delayCount == 0)) {
    repeating = true;
  }

  renderScreen(); // Render and display the entire screen
}
// =======================================================================


// Analog control
void modeAnalog() {
  if (ab_justPressed(RIGHT_BUTTON)) {
    valueInc(1);
    startButtonDelay();
  }
  else if (ab_justPressed(LEFT_BUTTON)) {
    valueDec(1);
    startButtonDelay();
  }
  else if (repeating && ab_pressed(RIGHT_BUTTON)) {
    valueInc(REPEAT_AMOUNT);
  }
  else if (repeating && ab_pressed(LEFT_BUTTON)) {
    valueDec(REPEAT_AMOUNT);
  }
  else if (ab_justPressed(DOWN_BUTTON)) {
    analogSelectInc();
  }
  else if (ab_justPressed(UP_BUTTON)) {
    analogSelectDec();
  }
  else if (repeating) {
    stopButtonRepeat();
  }
}

// Digital control
void modeDigital() {
  if (ab_justPressed(RIGHT_BUTTON) || ab_justPressed(LEFT_BUTTON)) {
    digitalState[digitalSelected] = (digitalState[digitalSelected] == RGB_ON) ?
                                     RGB_OFF : RGB_ON;
    arduboy.digitalWriteRGB(LEDpin[digitalSelected],
                            digitalState[digitalSelected]);
  }
  else if (ab_justPressed(DOWN_BUTTON)) {
    digitalSelectInc();
  }
  else if (ab_justPressed(UP_BUTTON)) {
    digitalSelectDec();
  }
}

// Reset to analog mode and turn all LEDs off
void reset() {
  digitalState[(unsigned char)(Color::RED)] = RGB_OFF;
  digitalState[(unsigned char)(Color::GREEN)] = RGB_OFF;
  digitalState[(unsigned char)(Color::BLUE)] = RGB_OFF;
  digitalSet();

  analogValue[(unsigned char)(Color::RED)] = 0;
  analogValue[(unsigned char)(Color::GREEN)] = 0;
  analogValue[(unsigned char)(Color::BLUE)] = 0;
  analogSet();

  digitalSelected = (unsigned char)(Color::RED);
  analogSelected = (unsigned char)(Color::RED);

  controlMode = ANALOG;
}

// Increment the selected analog LED value by the specified amount
// and update the LED
void valueInc(unsigned char amount) {
  if ((ANALOG_MAX - analogValue[analogSelected]) <= amount) {
    analogValue[analogSelected] = ANALOG_MAX;
  }
  else {
    analogValue[analogSelected] += amount;
  }

  arduboy.setRGBled(LEDpin[analogSelected], analogValue[analogSelected]);
}

// Decrement the selected analog LED value by the specified amount
// and update the LED
void valueDec(unsigned char amount) {
  if (analogValue[analogSelected] <= amount) {
    analogValue[analogSelected] = 0;
  }
  else {
    analogValue[analogSelected] -= amount;
  }

  arduboy.setRGBled(LEDpin[analogSelected], analogValue[analogSelected]);
}

// Select the next analog color index with wrap
void analogSelectInc() {
  selectInc(analogSelected);
}

// Select the previous analog color index with wrap
void analogSelectDec() {
  selectDec(analogSelected);
}

// Select the next digital color index with wrap
void digitalSelectInc() {
  selectInc(digitalSelected);
}

// Select the previous digital color index with wrap
void digitalSelectDec() {
  selectDec(digitalSelected);
}

// Select the next color index with wrap
void selectInc(unsigned char &index) {
  if (++index == (unsigned char)(Color::COUNT)) {
    index = 0;
  }
}

// Select the previous color index with wrap
void selectDec(unsigned char &index) {
  if (index == 0) {
    index = ((unsigned char)(Color::COUNT) - 1);
  }
  else {
    index--;
  }
}

// Update all LEDs in analog mode
void analogSet() {
  arduboy.setRGBled(analogValue[(unsigned char)(Color::RED)],
                    analogValue[(unsigned char)(Color::GREEN)],
                    analogValue[(unsigned char)(Color::BLUE)]);
}

// Update all LEDs in digital mode
void digitalSet() {
  arduboy.digitalWriteRGB(digitalState[(unsigned char)(Color::RED)],
                          digitalState[(unsigned char)(Color::GREEN)],
                          digitalState[(unsigned char)(Color::BLUE)]);
}

// Start the button auto-repeat delay
void startButtonDelay() {
  delayCount = DELAY_FRAMES;
  repeating = false;
}

// Stop the button auto-repeat or delay
void stopButtonRepeat() {
  delayCount = 0;
  repeating = false;
}

// Render and display the screen
void renderScreen() {
  ab_setCursor(12, 0);
  arduboy.print(F("RGB LED"));
  ab_setCursor(15, 56);
  arduboy.print(F("A:Mode   B:Reset"));
  ab_setCursor(74, 0);

  if (controlMode == ANALOG) {
    arduboy.print(F(" Analog"));
    drawAnalog(9, Color::RED, "Red:");
    drawAnalog(25, Color::GREEN, "Green:");
    drawAnalog(41, Color::BLUE, "Blue:");
  }
  else { // Digital
    arduboy.print(F("Digital"));
    drawDigital(9, Color::RED, "Red:");
    drawDigital(25, Color::GREEN, "Green:");
    drawDigital(41, Color::BLUE, "Blue:");
  }

  ab_display(CLEAR_BUFFER);
}

// Draw the information for one analog color
void drawAnalog(int y, Color color, const char* name) {
  unsigned char value = analogValue[(unsigned char)color];

  ab_setCursor(0, y);
  arduboy.print(name);
  ab_setCursor(42, y);
  printValue(value);
  if (analogSelected == (unsigned char)color) {
    arduboy.print(F(" <--"));
  }
  drawBar(y + 8, color, value);
}

// Draw the value bar for an analog color
void drawBar(int y, Color color, unsigned char value) {
  unsigned char barLength = value / 2;

  if (barLength == 0) {
    return;
  }

  if (analogSelected == (unsigned char)color) {
    ab_fillRect(0, y, barLength, 5);
  }
  else {
    ab_drawRect(0, y, barLength, 5);
  }
}

// Draw the information for one digital color
void drawDigital(int y, Color color, const char* name) {
  unsigned char state = digitalState[(unsigned char)color];

  ab_setCursor(34, y + 3);
  arduboy.print(name);
  ab_setCursor(76, y + 3);
  if (state == RGB_ON) {
    arduboy.print(F("ON "));
    ab_fillCircle(22, y + 6, 4);
  }
  else {
    arduboy.print(F("OFF"));
    ab_drawCircle(22, y + 6, 4);
  }

  if (digitalSelected == (unsigned char)color) {
    arduboy.print(F(" <--"));
    ab_drawRect(16, y, 13, 13);
  }
}

// Print a unsigned char in decimal and hex
void printValue(unsigned char val) {
  if (val < 100) {
    arduboy.print(' ');
  }
  if (val < 10) {
    arduboy.print(' ');
  }
  arduboy.print(val);

  arduboy.print(F("  0x"));
  if (val < 0x10) {
    arduboy.print('0');
  }
  arduboy.print(val, HEX);
}

