import re
from collections import defaultdict

class ParsedSketch:
    def __init__(self):
        self.raw_code = ""
        self.cleaned_code = ""

        self.includes = []
        self.functions = {}
        self.function_calls = defaultdict(set)

        self.bitmaps = []
        self.audio_scores = []

        self.libraries = {
            "arduboy": False,
            "arduboy2": False,
            "playtune": False
        }


def strip_comments(code):
    code = re.sub(r'//.*', '', code)
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.S)
    return code


def detect_libraries(code):
    return {
        "arduboy2": "Arduboy2" in code,
        "arduboy": "#include <Arduboy.h>" in code,
        "playtune": "Playtune" in code or "ArduboyPlaytune" in code
    }


def extract_includes(code):
    return re.findall(r'#include\s+[<"].*[>"]', code)


def extract_functions(code):
    functions = {}
    pattern = re.compile(r'\b(\w+)\s+(\w+)\s*\((.*?)\)\s*\{', re.S)

    pos = 0
    while True:
        m = pattern.search(code, pos)
        if not m:
            break

        name = m.group(2)
        start = m.end()

        depth = 1
        i = start

        while i < len(code) and depth > 0:
            if code[i] == '{':
                depth += 1
            elif code[i] == '}':
                depth -= 1
            i += 1

        functions[name] = code[m.start():i]
        pos = i

    return functions


def detect_calls(functions):
    call_map = defaultdict(set)
    pattern = re.compile(r'(\w+)\s*\(')

    for fname, body in functions.items():
        found = pattern.findall(body)
        for f in found:
            if f != fname:
                call_map[fname].add(f)

    return call_map


def extract_bitmaps(code):
    pattern = re.compile(
        r'const\s+(?:unsigned\s+)?char\s+(\w+)\s*\[\]\s*PROGMEM\s*=\s*\{(.*?)\};',
        re.S
    )

    result = []
    for name, data in pattern.findall(code):
        values = [v.strip() for v in data.split(",") if v.strip()]
        result.append({
            "name": name,
            "data": values
        })

    return result


def parse_code(code):
    parsed = ParsedSketch()
    parsed.raw_code = code

    cleaned = strip_comments(code)
    parsed.cleaned_code = cleaned

    parsed.includes = extract_includes(cleaned)
    parsed.functions = extract_functions(cleaned)
    parsed.function_calls = detect_calls(parsed.functions)

    parsed.bitmaps = extract_bitmaps(cleaned)
    parsed.libraries = detect_libraries(cleaned)

    return parsed
