import re


def _opt_fill_rect(code: str) -> str:
    pattern = re.compile(
        r'''
        for\s*\(\s*int\s+(?P<i>[A-Za-z_]\w*)\s*=\s*0\s*;\s*
            (?P=i)\s*<\s*(?P<w>[^;]+?)\s*;\s*(?P=i)\+\+\s*\)\s*
        \{\s*
            for\s*\(\s*int\s+(?P<j>[A-Za-z_]\w*)\s*=\s*0\s*;\s*
                (?P=j)\s*<\s*(?P<h>[^;]+?)\s*;\s*(?P=j)\+\+\s*\)\s*
            \{\s*
                ab_drawPixel\s*\(\s*
                    (?P<x>[^,]+?)\s*\+\s*(?P=i)\s*,\s*
                    (?P<y>[^,]+?)\s*\+\s*(?P=j)\s*,\s*
                    (?P<c>[^\)]+?)\s*
                \)\s*;\s*
            \}\s*
        \}
        ''',
        re.VERBOSE | re.DOTALL,
    )

    def repl(m):
        return (
            f"ab_fillRect({m.group('x').strip()}, {m.group('y').strip()}, "
            f"{m.group('w').strip()}, {m.group('h').strip()}, {m.group('c').strip()});"
        )

    return re.sub(pattern, repl, code)


def _opt_vline(code: str) -> str:
    pattern = re.compile(
        r'''
        for\s*\(\s*int\s+(?P<i>[A-Za-z_]\w*)\s*=\s*0\s*;\s*
            (?P=i)\s*<\s*(?P<h>[^;]+?)\s*;\s*(?P=i)\+\+\s*\)\s*
        \{\s*
            ab_drawPixel\s*\(\s*
                (?P<x>[^,]+?)\s*,\s*
                (?P<y>[^,]+?)\s*\+\s*(?P=i)\s*,\s*
                (?P<c>[^\)]+?)\s*
            \)\s*;\s*
        \}
        ''',
        re.VERBOSE | re.DOTALL,
    )

    def repl(m):
        return (
            f"ab_drawFastVLine({m.group('x').strip()}, {m.group('y').strip()}, "
            f"{m.group('h').strip()}, {m.group('c').strip()});"
        )

    return re.sub(pattern, repl, code)


def _opt_hline(code: str) -> str:
    pattern = re.compile(
        r'''
        for\s*\(\s*int\s+(?P<i>[A-Za-z_]\w*)\s*=\s*0\s*;\s*
            (?P=i)\s*<\s*(?P<w>[^;]+?)\s*;\s*(?P=i)\+\+\s*\)\s*
        \{\s*
            ab_drawPixel\s*\(\s*
                (?P<x>[^,]+?)\s*\+\s*(?P=i)\s*,\s*
                (?P<y>[^,]+?)\s*,\s*
                (?P<c>[^\)]+?)\s*
            \)\s*;\s*
        \}
        ''',
        re.VERBOSE | re.DOTALL,
    )

    def repl(m):
        return (
            f"ab_drawFastHLine({m.group('x').strip()}, {m.group('y').strip()}, "
            f"{m.group('w').strip()}, {m.group('c').strip()});"
        )

    return re.sub(pattern, repl, code)


def _opt_boolean_return(code: str) -> str:
    pattern = re.compile(
        r'''
        if\s*\(\s*(?P<cond>.*?)\s*\)\s*
        \{\s*return\s+true\s*;\s*\}\s*
        else\s*
        \{\s*return\s+false\s*;\s*\}
        ''',
        re.VERBOSE | re.DOTALL,
    )

    def repl(m):
        return f"return ({m.group('cond').strip()});"

    return re.sub(pattern, repl, code)


def _opt_boolean_assignment(code: str) -> str:
    pattern = re.compile(
        r'''
        if\s*\(\s*(?P<cond>.*?)\s*\)\s*
        \{\s*(?P<lhs>[A-Za-z_]\w*(?:\[[^\]]+\])?)\s*=\s*true\s*;\s*\}\s*
        else\s*
        \{\s*(?P=lhs)\s*=\s*false\s*;\s*\}
        ''',
        re.VERBOSE | re.DOTALL,
    )

    def repl(m):
        return f"{m.group('lhs').strip()} = ({m.group('cond').strip()});"

    return re.sub(pattern, repl, code)


def _opt_arithmetic_cleanup(code: str) -> str:
    replacements = [
        (r'\+\s*0\b', ''),
        (r'\b0\s*\+\s*', ''),
        (r'-\s*0\b', ''),
        (r'\*\s*1\b', ''),
        (r'\b1\s*\*\s*', ''),
        (r'/\s*1\b', ''),
    ]

    new = code
    for pattern, repl in replacements:
        new = re.sub(pattern, repl, new)
    return new


def _opt_force_inline_functions(code: str, function_names: list[str]) -> str:
    """
    For Mystic Balloon only:
    Mark a small set of hot helper functions as always_inline.
    This is much safer than macro-overriding the whole runtime.
    """
    for name in function_names:
        pattern = re.compile(
            rf'(?m)^(?P<indent>\s*)(?:static\s+)?void\s+{re.escape(name)}\s*\('
        )
        code = re.sub(
            pattern,
            rf'\g<indent>static inline __attribute__((always_inline)) void {name}(',
            code,
        )
    return code


def _opt_mystic_balloon(code: str) -> str:
    hot_helpers = [
        "windNoise",
        "kidHurt",
        "drawBalloonLives",
        "drawCoinHUD",
        "updateCamera",
        "checkInputs",
        "drawHUD",
        "setKid",
        "checkKid",
    ]
    return _opt_force_inline_functions(code, hot_helpers)


def _opt_small_cleanup_passes(code: str) -> str:
    new = code
    new = _opt_boolean_return(new)
    new = _opt_boolean_assignment(new)
    new = _opt_arithmetic_cleanup(new)
    return new


def optimize_generated_c(code: str, game_name: str | None = None) -> str:
    old = None
    new = code

    while old != new:
        old = new
        new = _opt_fill_rect(new)
        new = _opt_vline(new)
        new = _opt_hline(new)
        new = _opt_small_cleanup_passes(new)

    if game_name == "MYBL_AB":
        new = _opt_mystic_balloon(new)

    return new
