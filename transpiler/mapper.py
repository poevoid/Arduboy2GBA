import re

HEADER = """
#include "../runtime/arduboy_compat.h"
"""

MAPPINGS = {
    # Arduboy2 core
    "arduboy.drawPixel": "ab_drawPixel",
    "arduboy.drawBitmap": "ab_drawBitmap",
    "arduboy.drawSlowXYBitmap": "ab_drawBitmap",
    "arduboy.clear": "ab_clear",
    "arduboy.display": "ab_display",
    "arduboy.pollButtons": "ab_pollButtons",
    "arduboy.pressed": "ab_pressed",
    "arduboy.tone": "ab_tone",

    # Arduboy classic
    "arduboy.setPixel": "ab_drawPixel",
    "arduboy.drawScreen": "ab_display",
    "arduboy.clearDisplay": "ab_clear",
    "arduboy.getInput": "ab_pollButtons",

    # Graphics primitives
    "arduboy.drawFastHLine": "ab_drawFastHLine",
    "arduboy.drawFastVLine": "ab_drawFastVLine",
    "arduboy.drawRect": "ab_drawRect",
    "arduboy.fillRect": "ab_fillRect",
    "arduboy.fillScreen": "ab_fillScreen",
    "arduboy.setCursor": "ab_setCursor",
    "arduboy.print": "ab_print",

    # Common sprite APIs
    "Sprites::drawOverwrite": "ab_drawOverwrite",
    "Sprites::drawSelfMasked": "ab_drawSelfMasked",
    "Sprites::drawErase": "ab_drawErase",
    "Sprites::drawPlusMask": "ab_drawPlusMask",
    "Sprites::drawExternalMask": "ab_drawExternalMask",

    "sprites.drawOverwrite": "ab_drawOverwrite",
    "sprites.drawSelfMasked": "ab_drawSelfMasked",
    "sprites.drawErase": "ab_drawErase",
    "sprites.drawPlusMask": "ab_drawPlusMask",
    "sprites.drawExternalMask": "ab_drawExternalMask",

    # Playtune
    "tunes.playScore": "ab_playScore",
    "tunes.stopScore": "ab_stopScore",
}

INCLUDE_PATTERNS = [
    r'^\s*#include\s*<Arduboy2\.h>\s*$',
    r'^\s*#include\s*<Arduboy\.h>\s*$',
    r'^\s*#include\s*<ArduboyPlaytune\.h>\s*$',
    r'^\s*#include\s*<ArduboyPlayTunes\.h>\s*$',
    r'^\s*#include\s*<Playtune\.h>\s*$',
    r'^\s*#include\s*<Sprites\.h>\s*$',
]

DECLARATION_PATTERNS = [
    r'^\s*Arduboy2\s+\w+\s*;\s*$',
    r'^\s*Arduboy\s+\w+\s*;\s*$',
    r'^\s*ArduboyPlaytune\s+\w+\s*;\s*$',
    r'^\s*ArduboyPlayTunes\s+\w+\s*;\s*$',
    r'^\s*Playtune\s+\w+\s*;\s*$',
    r'^\s*Sprites\s+\w+\s*;\s*$',
]

def strip_arduino_specific_lines(code):
    for pattern in INCLUDE_PATTERNS:
        code = re.sub(pattern, '', code, flags=re.MULTILINE)

    for pattern in DECLARATION_PATTERNS:
        code = re.sub(pattern, '', code, flags=re.MULTILINE)

    code = re.sub(r'\b\w+\.begin\s*\(\s*\)\s*;', '', code)
    code = re.sub(r'\b\w+\.boot\s*\(\s*\)\s*;', '', code)
    code = re.sub(r'\b\w+\.bootLCD\s*\(\s*\)\s*;', '', code)
    code = re.sub(r'\b\w+\.audio\.on\s*\(\s*\)\s*;', '', code)
    code = re.sub(r'\b\w+\.audio\.off\s*\(\s*\)\s*;', '', code)

    return code

def normalize_structure(code):
    if "void setup()" not in code:
        code += "\nvoid setup() {}\n"

    if "void loop()" not in code:
        code += "\nvoid loop() {}\n"

    return code

def apply_mappings(code):
    for k in sorted(MAPPINGS.keys(), key=len, reverse=True):
        code = code.replace(k, MAPPINGS[k])
    return code

def map_code(parsed):
    code = parsed.raw_code
    code = strip_arduino_specific_lines(code)
    code = normalize_structure(code)
    code = apply_mappings(code)
    return HEADER + "\n" + code
