import re


def _opt_fill_rect(code: str) -> str:
    """
    Convert nested pixel loops like:

        for ( int i = 0; i < W; i++ ) {
          for ( int j = 0; j < H; j++ ) {
            ab_drawPixel(X+i, Y+j, C);
          }
        }

    into:

        ab_fillRect(X, Y, W, H, C);
    """

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
        x = m.group("x").strip()
        y = m.group("y").strip()
        w = m.group("w").strip()
        h = m.group("h").strip()
        c = m.group("c").strip()
        return f"ab_fillRect({x}, {y}, {w}, {h}, {c});"

    return re.sub(pattern, repl, code)


def _opt_vline(code: str) -> str:
    """
    Convert loops like:

        for ( int i = 0; i < H; i++ ) {
          ab_drawPixel(X, Y+i, C);
        }

    into:

        ab_drawFastVLine(X, Y, H, C);
    """

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
        x = m.group("x").strip()
        y = m.group("y").strip()
        h = m.group("h").strip()
        c = m.group("c").strip()
        return f"ab_drawFastVLine({x}, {y}, {h}, {c});"

    return re.sub(pattern, repl, code)


def _opt_hline(code: str) -> str:
    """
    Convert loops like:

        for ( int i = 0; i < W; i++ ) {
          ab_drawPixel(X+i, Y, C);
        }

    into:

        ab_drawFastHLine(X, Y, W, C);
    """

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
        x = m.group("x").strip()
        y = m.group("y").strip()
        w = m.group("w").strip()
        c = m.group("c").strip()
        return f"ab_drawFastHLine({x}, {y}, {w}, {c});"

    return re.sub(pattern, repl, code)


def optimize_generated_c(code: str) -> str:
    """
    Run a few safe pattern-based optimizations on already-mapped C code.
    Order matters: fillRect first, then line collapses.
    """
    old = None
    new = code

    while old != new:
        old = new
        new = _opt_fill_rect(new)
        new = _opt_vline(new)
        new = _opt_hline(new)

    return new
