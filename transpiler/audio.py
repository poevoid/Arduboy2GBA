import re


def normalize_audio_calls(code):
    replacements = [
        (r'arduboy\.tunes\.tone\s*\(', 'ab_tone('),
        (r'arduboy\.tunes\.playScore\s*\(', 'ab_playScore('),
        (r'arduboy\.tunes\.stopScore\s*\(', 'ab_stopScore('),

        (r'arduboy\.tone\s*\(', 'ab_tone('),
        (r'tunes\.playScore\s*\(', 'ab_playScore('),
        (r'tunes\.stopScore\s*\(', 'ab_stopScore('),
        (r'tunes\.tone\s*\(', 'ab_tone('),

        (r'playtune\.playScore\s*\(', 'ab_playScore('),
        (r'playtune\.stopScore\s*\(', 'ab_stopScore('),
        (r'playtune\.tone\s*\(', 'ab_tone('),

        (r'(?<![\.\w])tone\s*\(', 'ab_tone('),
    ]

    for pattern, replacement in replacements:
        code = re.sub(pattern, replacement, code)

    return code


def process_audio(parsed):
    code = parsed.raw_code
    code = normalize_audio_calls(code)
    return code
