import re

def normalize_audio_calls(code):
    replacements = {
        r'arduboy\.tone\s*\(': 'ab_tone(',
        r'\btone\s*\(': 'ab_tone(',
        r'tunes\.playScore\s*\(': 'ab_playScore(',
        r'tunes\.stopScore\s*\(': 'ab_stopScore(',
    }

    for pattern, replacement in replacements.items():
        code = re.sub(pattern, replacement, code)

    return code


def process_audio(parsed):
    code = parsed.raw_code
    code = normalize_audio_calls(code)
    return code
