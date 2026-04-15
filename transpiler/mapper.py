import re

HEADER = """
#include "arduboy_compat.h"
"""

DIRECT_MAPPINGS = {
    # Arduboy2 core
    "arduboy.drawPixel": "ab_drawPixel",
    "arduboy.drawBitmap": "ab_drawBitmap",
    "arduboy.drawSlowXYBitmap": "ab_drawBitmap",
    "arduboy.clear": "ab_clear",
    "arduboy.display": "ab_display",
    "arduboy.pollButtons": "ab_pollButtons",
    "arduboy.pressed": "ab_pressed",
    "arduboy.justPressed": "ab_justPressed",
    "arduboy.setCursor": "ab_setCursor",
    "arduboy.setTextSize": "ab_setTextSize",
    "arduboy.setTextWrap": "ab_setTextWrap",
    "arduboy.invert": "ab_invert",
    "arduboy.beginNoLogo": "ab_beginNoLogo",
    "arduboy.begin": "ab_begin",
    "arduboy.setFrameRate": "ab_setFrameRate",
    "arduboy.initRandomSeed": "ab_initRandomSeed",
    "arduboy.nextFrame": "ab_nextFrame",
    "arduboy.print": "ab_print",
    "arduboy.println": "ab_println",

    # Arduboy classic
    "arduboy.setPixel": "ab_drawPixel",
    "arduboy.drawScreen": "ab_display",
    "arduboy.clearDisplay": "ab_clear",
    "arduboy.getInput": "ab_pollButtons",
    "arduboy.blank": "ab_clear",

    # Graphics primitives
    "arduboy.drawFastHLine": "ab_drawFastHLine",
    "arduboy.drawFastVLine": "ab_drawFastVLine",
    "arduboy.drawRect": "ab_drawRect",
    "arduboy.fillRect": "ab_fillRect",
    "arduboy.fillScreen": "ab_fillScreen",

    # Sprites APIs
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

    # Audio
    "arduboy.tunes.tone": "ab_tone",
    "arduboy.tunes.playScore": "ab_playScore",
    "arduboy.tunes.stopScore": "ab_stopScore",
    "tunes.playScore": "ab_playScore",
    "tunes.stopScore": "ab_stopScore",
    "tunes.tone": "ab_tone",
}

INCLUDE_PATTERNS = [
    r'^\s*#include\s*[<"]Arduboy2\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduboy\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyPlaytune\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyPlayTunes\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Playtune\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Sprites\.h[>"]\s*$',
]

DECLARATION_PATTERNS = [
    r'^\s*Arduboy2\s+\w+\s*;\s*$',
    r'^\s*Arduboy2Base\s+\w+\s*;\s*$',
    r'^\s*Arduboy\s+\w+\s*;\s*$',
    r'^\s*ArduboyPlaytune\s+\w+\s*;\s*$',
    r'^\s*ArduboyPlayTunes\s+\w+\s*;\s*$',
    r'^\s*Playtune\s+\w+\s*;\s*$',
    r'^\s*Sprites\s+\w+\s*;\s*$',
]


def strip_arduino_specific_lines(code):
    for pattern in INCLUDE_PATTERNS:
        code = re.sub(pattern, "", code, flags=re.MULTILINE)

    for pattern in DECLARATION_PATTERNS:
        code = re.sub(pattern, "", code, flags=re.MULTILINE)

    return code


def normalize_arduino_types(code):
    code = re.sub(r"\bboolean\b", "bool", code)
    code = re.sub(r"\bbyte\b", "unsigned char", code)

    code = re.sub(r"\bconst\s+String\b", "const char*", code)
    code = re.sub(r"\bString\b", "char*", code)

    return code


def normalize_arduino_helpers(code):
    code = re.sub(r"\brandom\s*\(", "ab_random(", code)
    code = re.sub(r"\bdelay\s*\(", "ab_delay(", code)
    return code


def normalize_structure(code):
    if "void setup()" not in code:
        code += "\nvoid setup() {}\n"

    if "void loop()" not in code:
        code += "\nvoid loop() {}\n"

    return code


def apply_direct_mappings(code):
    for k in sorted(DIRECT_MAPPINGS.keys(), key=len, reverse=True):
        code = code.replace(k, DIRECT_MAPPINGS[k])
    return code


def generate_prototypes(code):
    pattern = re.compile(
        r'^\s*([A-Za-z_][\w\s\*\&]*?)\s+([A-Za-z_]\w*)\s*\((.*?)\)\s*\{',
        re.MULTILINE | re.DOTALL
    )

    keywords = {"if", "for", "while", "switch"}
    prototypes = []

    for m in pattern.finditer(code):
        ret_type = m.group(1).strip()
        name = m.group(2).strip()
        args = m.group(3).strip()

        if name in keywords:
            continue

        prototype = f"{ret_type} {name}({args});"
        if prototype not in prototypes:
            prototypes.append(prototype)

    return prototypes


def build_prelude(prototypes):
    lines = [
        HEADER.strip(),
        "",
        "#include <math.h>",
        "#include <string.h>",
        "",
    ]

    if prototypes:
        lines.extend(prototypes)
        lines.append("")

    return "\n".join(lines) + "\n"


def map_code(parsed):
    code = parsed.raw_code
    code = strip_arduino_specific_lines(code)
    code = normalize_arduino_types(code)
    code = normalize_arduino_helpers(code)
    code = normalize_structure(code)
    code = apply_direct_mappings(code)

    prototypes = generate_prototypes(code)
    prelude = build_prelude(prototypes)

    return prelude + "\n" + code
