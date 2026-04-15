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


def find_function_definitions(code):
    """
    Find actual function definitions only.

    Supports both:
      void foo()
      {
      }

    and:
      void foo() {
      }

    Avoids matching object declarations like:
      ArduboyPlaytune tunes(arduboy.audio.enabled);
    """
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
            args = m.group(3).strip()

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
            args = m.group(3).strip()

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
    code = normalize_arduino_helpers(code)
    code = normalize_structure(code)
    code = apply_direct_mappings(code)

    functions = find_function_definitions(code)
    prototypes = [f["prototype"] for f in functions]

    header_block = build_header_block()
    code = insert_prototypes_before_first_function(code, prototypes)

    return header_block + code
