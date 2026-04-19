import os
import re
from typing import List, Optional, Set


LOCAL_INCLUDE_RE = re.compile(
    r'^\s*#include\s*"([^"]+)"\s*$',
    re.MULTILINE,
)

SUPPORTED_LOCAL_EXTENSIONS = {
    ".h",
    ".hpp",
    ".hh",
    ".inc",
    ".ipp",
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".ino",
}


class IncludeExpansionError(RuntimeError):
    pass


def _is_supported_local_include(path: str) -> bool:
    _, ext = os.path.splitext(path)
    return ext.lower() in SUPPORTED_LOCAL_EXTENSIONS


def _resolve_local_include(
    include_name: str,
    current_dir: str,
    project_root_dir: str,
) -> Optional[str]:
    candidates: List[str] = [
        os.path.abspath(os.path.join(current_dir, include_name)),
        os.path.abspath(os.path.join(project_root_dir, include_name)),
    ]

    seen: Set[str] = set()
    for candidate in candidates:
        norm = os.path.normcase(os.path.normpath(candidate))
        if norm in seen:
            continue
        seen.add(norm)
        if os.path.isfile(candidate):
            return candidate

    return None


def _expand_file(
    file_path: str,
    project_root_dir: str,
    active_stack: Set[str],
    expanded_once: Set[str],
) -> str:
    real_path = os.path.realpath(file_path)

    if real_path in active_stack:
        raise IncludeExpansionError(
            f"Cyclic local include detected while expanding: {file_path}"
        )

    active_stack.add(real_path)

    try:
        with open(file_path, "r", encoding="utf-8") as f:
            source = f.read()

        current_dir = os.path.dirname(file_path)

        def repl(match: re.Match) -> str:
            include_name = match.group(1)

            # Only consider normal source/header-like files for local expansion.
            # Unknown extensions are left alone.
            if not _is_supported_local_include(include_name):
                return match.group(0)

            resolved = _resolve_local_include(
                include_name=include_name,
                current_dir=current_dir,
                project_root_dir=project_root_dir,
            )

            # Important:
            # If the include is not found locally, leave it untouched.
            # Many Arduboy/library headers use quoted includes internally,
            # and they are not sketch-local files we should inline.
            if resolved is None:
                return match.group(0)

            resolved_real = os.path.realpath(resolved)

            # Header guards / repeated includes:
            # only inline a given physical file once.
            if resolved_real in expanded_once:
                rel_name = os.path.relpath(resolved, project_root_dir)
                return f"\n/* SKIPPED DUPLICATE LOCAL INCLUDE: {rel_name} */\n"

            expanded_once.add(resolved_real)

            expanded = _expand_file(
                file_path=resolved,
                project_root_dir=project_root_dir,
                active_stack=active_stack,
                expanded_once=expanded_once,
            )

            rel_name = os.path.relpath(resolved, project_root_dir)
            return (
                f"\n/* BEGIN LOCAL INCLUDE: {rel_name} */\n"
                f"{expanded}\n"
                f"/* END LOCAL INCLUDE: {rel_name} */\n"
            )

        return LOCAL_INCLUDE_RE.sub(repl, source)

    finally:
        active_stack.remove(real_path)


def expand_local_includes(entry_file: str) -> str:
    entry_file = os.path.abspath(entry_file)
    project_root_dir = os.path.dirname(entry_file)

    expanded_once: Set[str] = {os.path.realpath(entry_file)}

    return _expand_file(
        file_path=entry_file,
        project_root_dir=project_root_dir,
        active_stack=set(),
        expanded_once=expanded_once,
    )
