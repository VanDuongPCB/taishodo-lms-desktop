#!/usr/bin/env python3
"""
Generate Qt resource files (.qrc) into Sources/Resources for the asset folders
shipped with LMS, so they are compiled into the executable instead of being
copied next to it.

    Sources/Resources/themes.qrc          prefix /themes   -> :/themes/<Theme>/css/button.css ...
    Sources/Resources/default_files.qrc   prefix /default  -> :/default/LMS.db

Each <file> entry points to the real asset (relative to Sources/Resources) and
carries an alias so the resource path stays short. qrc has no wildcards, so run
the script again after adding, renaming or removing an asset file:

    python Sources/tools/generate_qrc.py
"""

import os
from pathlib import Path
from xml.sax.saxutils import escape, quoteattr

ROOT = Path(__file__).resolve().parent.parent          # .../Sources
OUT_DIR = ROOT / "Resources"

TARGETS = [
    # (asset folder, qrc file name, prefix, file patterns)
    (ROOT / "Resources" / "themes", "themes.qrc", "/themes",
     ("*.css", "*.qss", "*.json", "*.png", "*.jpg", "*.jpeg", "*.svg", "*.gif", "*.ttf", "*.otf")),
    (ROOT / "Default Files", "default_files.qrc", "/default",
     ("*.db", "*.json", "*.txt", "*.csv")),
]


def generate(folder: Path, qrc_name: str, prefix: str, patterns) -> int:
    files = set()
    for pattern in patterns:
        files.update(p for p in folder.rglob(pattern) if p.is_file())
    files = sorted(files, key=lambda p: p.relative_to(folder).as_posix().lower())

    lines = ['<!DOCTYPE RCC>', '<RCC version="1.0">', f'<qresource prefix="{prefix}">']
    for f in files:
        alias = f.relative_to(folder).as_posix()
        rel = Path(os.path.relpath(f, OUT_DIR)).as_posix()
        lines.append(f"    <file alias={quoteattr(alias)}>{escape(rel)}</file>")
    lines += ["</qresource>", "</RCC>", ""]

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / qrc_name).write_text("\n".join(lines), encoding="utf-8", newline="\n")
    return len(files)


def main() -> None:
    for folder, qrc_name, prefix, patterns in TARGETS:
        if not folder.is_dir():
            print(f"skip {folder}: not found")
            continue
        n = generate(folder, qrc_name, prefix, patterns)
        print(f"{(OUT_DIR / qrc_name).relative_to(ROOT).as_posix()}: {n} files, prefix {prefix}")


if __name__ == "__main__":
    main()
