import re
from dataclasses import dataclass

@dataclass
class ParsedSketch:
    raw_code: str
    includes: list
    functions: list
    bitmaps: dict


def strip_comments_for_analysis(code: str) -> str:
    code = re.sub(r"/\*.*?\*/", "", code, flags=re.S)
    code = re.sub(r"//.*", "", code)
    return code


def parse_code(source: str) -> ParsedSketch:
    clean = strip_comments_for_analysis(source)

    includes = re.findall(r'#include\s+[<"].*[>"]', clean)
    functions = re.findall(r'\bvoid\s+(\w+)\s*\(', clean)

    bitmaps = {}
    for match in re.finditer(r'const\s+unsigned\s+char\s+(\w+)\[\]', clean):
        bitmaps[match.group(1)] = {}

    return ParsedSketch(
        raw_code=source,  # keep original intact
        includes=includes,
        functions=functions,
        bitmaps=bitmaps
    )
