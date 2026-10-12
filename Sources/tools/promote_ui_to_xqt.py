#!/usr/bin/env python3
"""
Switch the LMS user interface from plain Qt widgets to the xQt X* classes.

1. *.ui files: every nested <widget class="QXxx"> becomes class="XXxx" and a
   matching <customwidget> entry (promotion) is added, so Qt Designer and uic
   both understand it. Top-level widgets (QMainWindow / QDialog) cannot be
   promoted in a .ui file and are left alone.

2. *.h / *.cpp: the Nx* windows and dialogs derive from XMainWindow / XDialog
   instead of QMainWindow / QDialog, and NxTableView derives from XTableView.
   Includes are added and constructor initializer lists updated.

Line endings of every file are preserved (the repository uses CRLF).
The script is idempotent: running it again changes nothing.

    python Sources/tools/promote_ui_to_xqt.py            # apply
    python Sources/tools/promote_ui_to_xqt.py --check    # report only
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent          # .../Sources
UI_DIR = ROOT / "Desktop"

# Qt class -> X class, for widgets nested inside .ui files.
# Containers need <container>1</container> so Designer accepts children.
UI_WIDGETS = {
    "QLabel":         ("XLabel", False),
    "QLineEdit":      ("XLineEdit", False),
    "QWidget":        ("XWidget", True),
    "QPushButton":    ("XPushButton", False),
    "QCheckBox":      ("XCheckBox", False),
    "QComboBox":      ("XComboBox", False),
    "QToolBar":       ("XToolBar", True),
    "QSpinBox":       ("XSpinBox", False),
    "QDoubleSpinBox": ("XDoubleSpinBox", False),
    "QGroupBox":      ("XGroupBox", True),
    "QToolButton":    ("XToolButton", False),
    "QPlainTextEdit": ("XPlainTextEdit", False),
    "QTabWidget":     ("XTabWidget", True),
    "QScrollArea":    ("XScrollArea", True),
    "QProgressBar":   ("XProgressBar", False),
}

# Top-level classes: handled in C++ (base class of the Nx* class).
BASE_CLASSES = {
    "QMainWindow": "XMainWindow",
    "QDialog":     "XDialog",
    "QTableView":  "XTableView",
}


def read(path: Path):
    """Return (text with \\n, original newline) so the file's line endings survive."""
    raw = path.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    return raw.decode("utf-8").replace("\r\n", "\n"), newline


def write(path: Path, text: str, newline: str) -> None:
    path.write_bytes(text.replace("\n", newline).encode("utf-8"))


def convert_ui(path: Path, check: bool) -> int:
    text, newline = read(path)
    original = text

    # The first <widget ...> is the top-level one; everything after it is nested.
    top = re.search(r'<widget class="[^"]*"', text)
    if not top:
        return 0
    head, body = text[:top.end()], text[top.end():]

    used = []
    for qclass, (xclass, _) in UI_WIDGETS.items():
        pattern = f'<widget class="{qclass}"'
        if pattern in body:
            body = body.replace(pattern, f'<widget class="{xclass}"')
            used.append(qclass)
        if f'<widget class="{xclass}"' in body and qclass not in used:
            used.append(qclass)            # already converted earlier: still needs customwidget
    text = head + body

    # customwidget entries
    entries = []
    for qclass in used:
        xclass, container = UI_WIDGETS[qclass]
        if f"<class>{xclass}</class>" in text:
            continue
        entries.append(
            "  <customwidget>\n"
            f"   <class>{xclass}</class>\n"
            f"   <extends>{qclass}</extends>\n"
            f"   <header>{xclass}.h</header>\n"
            + ("   <container>1</container>\n" if container else "")
            + "  </customwidget>\n"
        )
    if entries:
        block = "".join(entries)
        if "</customwidgets>" in text:
            text = text.replace(" </customwidgets>", block + " </customwidgets>", 1)
        else:
            anchor = re.search(r"^ <resources|^ <connections|^</ui>", text, re.M)
            text = text[:anchor.start()] + " <customwidgets>\n" + block + " </customwidgets>\n" + text[anchor.start():]

    if text == original:
        return 0
    if not check:
        write(path, text, newline)
    return sum(body.count(f'<widget class="{UI_WIDGETS[q][0]}"') for q in used)


def convert_sources(check: bool) -> list:
    """Change base classes in Nx*.h / Nx*.cpp. Returns list of (file, old, new)."""
    changes = []
    for header in sorted(UI_DIR.glob("Nx*.h")):
        text, newline = read(header)
        original = text
        for qbase, xbase in BASE_CLASSES.items():
            m = re.search(rf"^class\s+(\w+)\s*:\s*public\s+{qbase}\b", text, re.M)
            if not m:
                continue
            text = text[:m.start()] + f"class {m.group(1)} : public {xbase}" + text[m.end():]
            # include the X header right after the Q header include (or after #pragma once)
            inc = re.search(rf'^#include\s+["<]{qbase}[">]\s*$', text, re.M)
            if f'#include "{xbase}.h"' not in text:
                if inc:
                    text = text[:inc.end()] + f'\n#include "{xbase}.h"' + text[inc.end():]
                else:
                    text = text.replace("#pragma once", f'#pragma once\n#include "{xbase}.h"', 1)
            changes.append((header.name, qbase, xbase))

            cpp = header.with_suffix(".cpp")
            if cpp.exists():
                ctext, cnewline = read(cpp)
                corig = ctext
                # constructor initializer: ": QMainWindow( parent )" / ":QMainWindow(parent)"
                ctext = re.sub(rf"(:\s*|,\s*){qbase}(\s*\(\s*parent\s*\))", rf"\g<1>{xbase}\g<2>", ctext)
                # explicit base calls: QMainWindow::eventFilter -> XMainWindow::eventFilter
                ctext = re.sub(rf"\b{qbase}::(?=\w+\s*\()", f"{xbase}::", ctext)
                # keep Qt signal references like &QTableView::pressed untouched
                ctext = ctext.replace(f"&{xbase}::", f"&{qbase}::")
                if ctext != corig:
                    if not check:
                        write(cpp, ctext, cnewline)
                    changes.append((cpp.name, qbase, xbase))
        if text != original and not check:
            write(header, text, newline)
    return changes


def main() -> int:
    check = "--check" in sys.argv
    total = 0
    for ui in sorted(UI_DIR.glob("*.ui")):
        n = convert_ui(ui, check)
        if n:
            print(f"{'would update' if check else 'updated'} {ui.name}: {n} widget(s)")
            total += n
    changes = convert_sources(check)
    for name, old, new in changes:
        print(f"{'would update' if check else 'updated'} {name}: {old} -> {new}")
    print(f"\n{total} widget instance(s) in .ui files, {len(changes)} source file(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
