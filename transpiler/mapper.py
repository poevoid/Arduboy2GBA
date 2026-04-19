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
    "arduboy.notPressed": "ab_notPressed",
    "arduboy.setCursor": "ab_setCursor",
    "arduboy.setTextSize": "ab_setTextSize",
    "arduboy.setTextWrap": "ab_setTextWrap",
    "arduboy.invert": "ab_invert",
    "arduboy.beginNoLogo": "ab_beginNoLogo",
    "arduboy.begin": "ab_begin",
    "arduboy.boot": "ab_begin",
    "arduboy.bootLogoSpritesSelfMasked": "ab_bootLogoSpritesSelfMasked",
    "arduboy.setFrameRate": "ab_setFrameRate",
    "arduboy.initRandomSeed": "ab_initRandomSeed",
    "arduboy.nextFrame": "ab_nextFrame",
    "arduboy.everyXFrames": "ab_everyXFrames",

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
    "arduboy.drawCircle": "ab_drawCircle",
    "arduboy.fillCircle": "ab_fillCircle",
    "arduboy.drawLine": "ab_drawLine",
    "arduboy.drawRoundRect": "ab_drawRoundRect",

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

    # Audio objects
    "arduboy.tunes.tone": "ab_tone",
    "arduboy.tunes.playScore": "ab_playScore",
    "arduboy.tunes.stopScore": "ab_stopScore",
    "tunes.playScore": "ab_playScore",
    "tunes.stopScore": "ab_stopScore",
    "tunes.tone": "ab_tone",
    "sound.tone": "ab_tone",

    # Math helpers
    "radians(": "ab_radians(",
}

INCLUDE_PATTERNS = [
    # Top-level Arduboy headers
    r'^\s*#include\s*[<"]Arduboy2\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduboy\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyPlaytune\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyPlayTunes\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyTones\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Playtune\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Sprites\.h[>"]\s*$',

    # Common Arduboy internal/library headers that can appear via quoted includes
    r'^\s*#include\s*[<"]Arduboy2Core\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduboy2Audio\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduboy2Beep\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduboy2CoreExt\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ArduboyFX\.h[>"]\s*$',
    r'^\s*#include\s*[<"]ATMlib\.h[>"]\s*$',
    r'^\s*#include\s*[<"]EEPROM\.h[>"]\s*$',
    r'^\s*#include\s*[<"]avr/pgmspace\.h[>"]\s*$',
    r'^\s*#include\s*[<"]avr/eeprom\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Arduino\.h[>"]\s*$',
    r'^\s*#include\s*[<"]WString\.h[>"]\s*$',
    r'^\s*#include\s*[<"]Print\.h[>"]\s*$',
]

DECLARATION_PATTERNS = [
    r'^\s*Arduboy2\s+\w+\s*;\s*$',
    r'^\s*Arduboy2Base\s+\w+\s*;\s*$',
    r'^\s*Arduboy\s+\w+\s*;\s*$',
    r'^\s*Sprites\s+\w+\s*;\s*$',
    r'^\s*ArduboyTones\s+\w+\s*\([^;]*\)\s*;\s*$',
]


def strip_arduino_specific_lines(code):
    for pattern in INCLUDE_PATTERNS:
        code = re.sub(pattern, "", code, flags=re.MULTILINE)

    for pattern in DECLARATION_PATTERNS:
        code = re.sub(pattern, "", code, flags=re.MULTILINE)

    # Local headers may contain pragma once after include expansion.
    code = re.sub(r'^\s*#pragma\s+once\s*$', "", code, flags=re.MULTILINE)

    return code


def normalize_arduino_types(code):
    code = re.sub(r"\bboolean\b", "bool", code)
    code = re.sub(r"\bbyte\b", "unsigned char", code)

    code = re.sub(r"\bconst\s+String\b", "const char*", code)
    code = re.sub(r"\bString\b", "char*", code)

    return code


def normalize_arduino_binary_literals(code):
    # Convert Arduino-style binary constants like B00110101 into standard C++ literals.
    code = re.sub(r"\bB([01]{8})\b", r"0b\1", code)
    return code


def normalize_pgm_function_pointer_reads(code):
    # On AVR, pgm_read_word() is often used to fetch function pointers from PROGMEM.
    # On GBA/ARM pointers are 32-bit, so that truncates them and breaks dispatch tables.
    # Rewrite only the "take address of indexed table entry" pattern to pgm_read_ptr().
    code = re.sub(
        r"\bpgm_read_word\s*\(\s*&\s*([A-Za-z_]\w*\s*\[[^]]+\])\s*\)",
        r"pgm_read_ptr(&\1)",
        code,
    )
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


def strip_default_args(arg_string):
    if not arg_string.strip():
        return arg_string

    parts = []
    current = []
    depth_paren = 0
    depth_angle = 0

    for ch in arg_string:
        if ch == ',' and depth_paren == 0 and depth_angle == 0:
            parts.append(''.join(current))
            current = []
            continue

        current.append(ch)

        if ch == '(':
            depth_paren += 1
        elif ch == ')':
            depth_paren = max(0, depth_paren - 1)
        elif ch == '<':
            depth_angle += 1
        elif ch == '>':
            depth_angle = max(0, depth_angle - 1)

    parts.append(''.join(current))

    cleaned_parts = []
    for part in parts:
        cleaned = re.sub(r"\s*=\s*.+$", "", part.strip())
        cleaned_parts.append(cleaned)

    return ", ".join(cleaned_parts)


def find_function_definitions(code):
    functions = []
    seen = set()
    keywords = {"if", "for", "while", "switch", "return"}

    lines = code.splitlines()
    offset = 0

    multi_line_sig = re.compile(
        r'^\s*([A-Za-z_~][\w\s\*\&\:<>]*?)\s+([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*$'
    )
    same_line_sig = re.compile(
        r'^\s*([A-Za-z_~][\w\s\*\&\:<>]*?)\s+([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{\s*$'
    )

    i = 0
    while i < len(lines):
        line = lines[i]
        line_start = offset

        m = same_line_sig.match(line)
        if m:
            ret_type = m.group(1).strip()
            name = m.group(2).strip()
            args = strip_default_args(m.group(3).strip())

            if name not in keywords:
                prototype = f"{ret_type} {name}({args});"
                if prototype not in seen:
                    seen.add(prototype)
                    functions.append({
                        "prototype": prototype,
                        "line_index": i,
                        "char_index": line_start,
                    })

            offset += len(line) + 1
            i += 1
            continue

        m = multi_line_sig.match(line)
        if m:
            ret_type = m.group(1).strip()
            name = m.group(2).strip()
            args = strip_default_args(m.group(3).strip())

            if name not in keywords:
                j = i + 1
                next_offset = offset + len(line) + 1

                while j < len(lines) and lines[j].strip() == "":
                    next_offset += len(lines[j]) + 1
                    j += 1

                if j < len(lines) and lines[j].strip() == "{":
                    prototype = f"{ret_type} {name}({args});"
                    if prototype not in seen:
                        seen.add(prototype)
                        functions.append({
                            "prototype": prototype,
                            "line_index": i,
                            "char_index": line_start,
                        })

            offset += len(line) + 1
            i += 1
            continue

        offset += len(line) + 1
        i += 1

    return functions


def build_header_block():
    return '\n'.join([
        HEADER.strip(),
        "",
        "#include <math.h>",
        "#include <string.h>",
        "#include <stdio.h>",
        "",
    ])


def insert_prototypes_before_first_function(code, prototypes):
    if not prototypes:
        return code

    functions = find_function_definitions(code)
    if not functions:
        return code

    insert_at = functions[0]["char_index"]
    proto_block = "\n".join(prototypes) + "\n\n"
    return code[:insert_at] + proto_block + code[insert_at:]


def map_code(parsed):
    code = parsed.raw_code
    code = strip_arduino_specific_lines(code)
    code = normalize_arduino_types(code)
    code = normalize_arduino_binary_literals(code)
    code = normalize_pgm_function_pointer_reads(code)
    code = normalize_arduino_helpers(code)
    code = normalize_structure(code)
    code = apply_direct_mappings(code)

    functions = find_function_definitions(code)
    prototypes = [f["prototype"] for f in functions]

    header_block = build_header_block()
    code = insert_prototypes_before_first_function(code, prototypes)

    return header_block + code
