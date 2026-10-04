#!/usr/bin/env python3
"""
Interactive CMake project generator for LMS.

Run without arguments for the interactive mode:

    python generator.py

    1. A table of CMake options is shown. Valid values are green, invalid red.
    2. If everything is green the menu offers: Reconfigure / Run CMake / Quit
       (default: Run CMake). If something is red the default is Reconfigure.
    3. Reconfigure walks through every option, shows the available choices
       as a numbered list and asks for a value. At the end the menu is shown
       again.

Settings are remembered in <root>/.generator.json.

Non-interactive use (CI): pass --yes. The table is printed, CMake runs if every
option is valid, otherwise the script exits with code 2.

    python generator.py --qt C:/Qt/6.8.0/msvc2022_64 --version 25.1.2 --yes
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent   # .../LMS/Sources  (root CMakeLists.txt)
ROOT_DIR = SCRIPT_DIR.parent                   # .../LMS
SETTINGS_FILE = ROOT_DIR / ".generator.json"

DEFAULT_VERSION = "99.99.99"
DEFAULT_ARCH = "x64"

QT_MODULES = ("Core", "Gui", "Widgets", "Network", "SerialPort", "Sql", "Concurrent")
QT_ROOTS = ("C:/Qt", "D:/Qt", "E:/Qt")                       # official layout: <root>/<version>/<kit>
QT_THIRDPARTY_ROOTS = ("C:/_ThirdParty/libqt", "D:/_ThirdParty/libqt", "E:/_ThirdParty/libqt")  # team layout: <root>/<os>/<arch>

VS_YEAR_TO_MAJOR = {"2017": 15, "2019": 16, "2022": 17, "2026": 18}
VS_MAJOR_TO_YEAR = {v: k for k, v in VS_YEAR_TO_MAJOR.items()}
VS_EDITIONS = ("Enterprise", "Professional", "Community", "Preview", "BuildTools")
NINJA_GENERATORS = ("Ninja", "Ninja Multi-Config")
ARCHS = ("x64", "Win32", "ARM64")


# ---------------------------------------------------------------------------
# Console colors
# ---------------------------------------------------------------------------
class Color:
    enabled = True
    RESET = "\033[0m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    CYAN = "\033[36m"

    @classmethod
    def setup(cls, force_off=False):
        cls.enabled = not force_off and sys.stdout.isatty() and not os.environ.get("NO_COLOR")
        if cls.enabled and os.name == "nt":
            try:
                import ctypes
                kernel32 = ctypes.windll.kernel32
                handle = kernel32.GetStdHandle(-11)
                mode = ctypes.c_uint32()
                if kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
                    kernel32.SetConsoleMode(handle, mode.value | 0x0004)
            except Exception:
                pass

    @classmethod
    def paint(cls, text, *codes):
        if not cls.enabled:
            return text
        return "".join(codes) + text + cls.RESET


def green(t): return Color.paint(t, Color.GREEN)
def red(t): return Color.paint(t, Color.RED)
def yellow(t): return Color.paint(t, Color.YELLOW)
def cyan(t): return Color.paint(t, Color.CYAN)
def bold(t): return Color.paint(t, Color.BOLD)
def dim(t): return Color.paint(t, Color.DIM)


def visible_len(text):
    return len(re.sub(r"\033\[[0-9;]*m", "", text))


def pad(text, width):
    return text + " " * max(0, width - visible_len(text))


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def run_capture(cmd, cwd=None):
    try:
        return subprocess.check_output(cmd, cwd=cwd, text=True, stderr=subprocess.DEVNULL).strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def ask(prompt):
    """Read a line. Returns None on EOF / Ctrl+C."""
    try:
        return input(prompt).strip().strip('"')
    except (EOFError, KeyboardInterrupt):
        print()
        return None


def norm_path(value):
    return Path(value).expanduser().resolve().as_posix() if value else ""


def is_vs_generator(name):
    return bool(name) and name.startswith("Visual Studio")


# ---------------------------------------------------------------------------
# Qt
# ---------------------------------------------------------------------------
def qt_config(qt_dir):
    return Path(qt_dir) / "lib" / "cmake" / "Qt6" / "Qt6Config.cmake"


def qt_missing_modules(qt_dir):
    return [m for m in QT_MODULES if not (Path(qt_dir) / "lib" / "cmake" / f"Qt6{m}").is_dir()]


def qt_kit_from_path(value):
    """Accept a kit dir, or a path inside it (e.g. .../lib/cmake/Qt6), return the kit dir."""
    if not value:
        return ""
    p = Path(value).expanduser()
    for candidate in (p, p.parent.parent.parent):
        if qt_config(candidate).exists():
            return candidate.resolve().as_posix()
    return ""


def detect_qt_kits():
    """Return list of (kit_dir, description)."""
    found = []

    def add(path, desc):
        kit = qt_kit_from_path(path)
        if kit and kit not in [k for k, _ in found]:
            found.append((kit, desc))

    for var in ("Qt6_DIR", "QT_DIR", "QTDIR", "CMAKE_PREFIX_PATH"):
        for p in os.environ.get(var, "").split(os.pathsep):
            if p:
                add(p, var)

    for root in QT_ROOTS:
        root_path = Path(root)
        if not root_path.is_dir():
            continue
        versions = sorted((d for d in root_path.glob("6.*") if d.is_dir()),
                          key=lambda d: [int(x) if x.isdigit() else 0 for x in d.name.split(".")],
                          reverse=True)
        for ver in versions:
            for kit in sorted(ver.iterdir()):
                if kit.is_dir() and qt_config(kit).exists():
                    add(kit, f"Qt {ver.name}")

    for root in QT_THIRDPARTY_ROOTS:
        root_path = Path(root)
        if not root_path.is_dir():
            continue
        for kit in sorted(root_path.glob("*/*")):
            if kit.is_dir() and qt_config(kit).exists():
                add(kit, "third-party")
    return found


# ---------------------------------------------------------------------------
# Visual Studio / git
# ---------------------------------------------------------------------------
def detect_git_version():
    tag = run_capture(["git", "describe", "--tags", "--abbrev=0"], cwd=ROOT_DIR)
    if tag and re.fullmatch(r"\d+\.\d+\.\d+", tag):
        return tag
    return ""


def detect_visual_studios():
    """
    Return a list of dicts {generator, instance, path}.
    `instance` is set only when the installation is not registered with the
    Visual Studio Installer; CMake then needs CMAKE_GENERATOR_INSTANCE.
    """
    found = []
    seen = set()

    pf = Path(os.environ.get("ProgramFiles", r"C:\Program Files"))
    pf86 = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))

    vswhere = pf86 / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if vswhere.exists():
        out = run_capture([str(vswhere), "-all", "-prerelease", "-products", "*",
                           "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                           "-format", "json"])
        if out:
            try:
                for inst in json.loads(out):
                    version = inst.get("installationVersion", "")
                    path = inst.get("installationPath", "")
                    major = int(version.split(".")[0]) if version else 0
                    year = VS_MAJOR_TO_YEAR.get(major)
                    if year and path and path.lower() not in seen:
                        seen.add(path.lower())
                        found.append({"generator": f"Visual Studio {major} {year}",
                                      "instance": None, "path": path})
            except (ValueError, AttributeError):
                pass

    for root in (pf, pf86):
        base = root / "Microsoft Visual Studio"
        for year in sorted(VS_YEAR_TO_MAJOR, reverse=True):
            for edition in VS_EDITIONS:
                install_dir = base / year / edition
                if str(install_dir).lower() in seen:
                    continue
                ini_path = install_dir / "Common7" / "IDE" / "devenv.isolation.ini"
                if not ini_path.exists():
                    continue
                version = ""
                try:
                    for line in ini_path.read_text(encoding="utf-8-sig", errors="ignore").splitlines():
                        m = re.match(r"\s*InstallationVersion\s*=\s*([\d.]+)", line)
                        if m:
                            version = m.group(1)
                            break
                except OSError:
                    pass
                if not version:
                    continue
                seen.add(str(install_dir).lower())
                major = VS_YEAR_TO_MAJOR[year]
                found.append({"generator": f"Visual Studio {major} {year}",
                              "instance": f"{install_dir.as_posix()},version={version}",
                              "path": str(install_dir)})
    return found


# ---------------------------------------------------------------------------
# Settings
# ---------------------------------------------------------------------------
KEYS = ("qt", "generator", "vs_instance", "arch", "version", "ide_dir", "build_dir", "install_dir")


def default_settings():
    return {
        "qt": "",
        "generator": "",
        "vs_instance": "",
        "arch": DEFAULT_ARCH,
        "version": detect_git_version() or DEFAULT_VERSION,
        "ide_dir": (ROOT_DIR / "_ide").as_posix(),
        "build_dir": (ROOT_DIR / "_build").as_posix(),
        "install_dir": (ROOT_DIR / "_install").as_posix(),
    }


def load_settings():
    try:
        data = json.loads(SETTINGS_FILE.read_text(encoding="utf-8"))
        return {k: str(data.get(k, "")) for k in KEYS if k in data}
    except (OSError, ValueError):
        return {}


def save_settings(s):
    try:
        SETTINGS_FILE.write_text(json.dumps({k: s[k] for k in KEYS}, indent=2) + "\n", encoding="utf-8")
        return True
    except OSError as e:
        print(red(f"Could not save settings: {e}"))
        return False


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------
class Row:
    def __init__(self, key, label, value, ok, note=""):
        self.key, self.label, self.value, self.ok, self.note = key, label, value, ok, note


def validate_qt(qt_dir):
    if not qt_dir:
        return False, "not set"
    path = Path(qt_dir).expanduser()
    if not path.exists():
        return False, f"directory does not exist: {qt_dir}"
    if not path.is_dir():
        return False, f"not a directory: {qt_dir}"
    if not qt_config(path).exists():
        return False, "lib/cmake/Qt6/Qt6Config.cmake not found (not a Qt kit directory)"
    missing = qt_missing_modules(qt_dir)
    if missing:
        return False, "missing Qt modules: " + ", ".join(missing)
    return True, ""


def validate(s, vs_list):
    rows = []

    qt_ok, note = validate_qt(s["qt"])
    rows.append(Row("qt", "Qt dir", s["qt"], qt_ok, note))

    gen = s["generator"]
    if not gen:
        gen_ok, note = False, "not set"
    elif is_vs_generator(gen):
        detected = any(v["generator"] == gen for v in vs_list)
        gen_ok = detected or bool(s["vs_instance"])
        note = "" if gen_ok else "Visual Studio not detected (set VS instance)"
    elif gen in NINJA_GENERATORS:
        gen_ok = bool(shutil.which("ninja"))
        note = "" if gen_ok else "ninja not found on PATH"
    else:
        gen_ok, note = True, ""
    rows.append(Row("generator", "CMake generator", gen, gen_ok, note))

    if is_vs_generator(gen):
        rows.append(Row("vs_instance", "VS instance", s["vs_instance"] or "(CMake default)", True))
        arch_ok = s["arch"] in ARCHS
        rows.append(Row("arch", "Architecture", s["arch"], arch_ok, "" if arch_ok else f"one of {', '.join(ARCHS)}"))

    ver_ok = bool(re.fullmatch(r"\d+\.\d+\.\d+", s["version"]))
    rows.append(Row("version", "Version", s["version"], ver_ok, "" if ver_ok else "expected X.Y.Z"))

    for key, label in (("ide_dir", "IDE dir"), ("build_dir", "Build dir"), ("install_dir", "Install dir")):
        v = s[key]
        if not v:
            ok, note = False, "not set"
        elif not Path(v).is_absolute():
            ok, note = False, "must be an absolute path"
        elif Path(v).resolve() == SCRIPT_DIR.resolve():
            ok, note = False, "must not be the source directory"
        else:
            ok, note = True, ""
        rows.append(Row(key, label, v, ok, note))

    return rows


def print_table(rows):
    label_w = max(len(r.label) for r in rows)
    print()
    print("  " + bold(pad("Option", label_w)) + "  " + bold("Value"))
    print("  " + "-" * label_w + "  " + "-" * 60)
    for r in rows:
        value = r.value if r.value else "<not set>"
        print("  " + pad(r.label, label_w) + "  " + (green(value) if r.ok else red(value)))
    print()


# ---------------------------------------------------------------------------
# Interactive configuration
# ---------------------------------------------------------------------------
def choose(title, current, choices, default="", allow_custom=True, allow_empty=False, validator=None):
    """
    Prompt layout:
        Specify <title>:

        Current: <value>      (or Default: <value>)
        1) choice
        2) choice
        Enter (q - quit):

    Enter keeps the current value (or takes the default when nothing is set).
    An invalid answer is reported and the prompt is repeated.
    Returns the chosen value, or None if the user quit (q) or input ended (EOF).
    Every accepted answer is saved immediately by the caller, so quitting keeps progress.
    """
    print(bold(f"\nSpecify {title}:"))
    print()
    if current:
        print(f"Current: {cyan(current)}")
    elif default:
        print(f"Default: {cyan(default)}")
    else:
        print(f"Current: {dim('<not set>')}")
    for i, (val, desc) in enumerate(choices, 1):
        shown = val if val else dim("<empty>")
        print(f"{i}) {shown}" + (dim(f"   {desc}") if desc else ""))

    while True:
        answer = ask("Enter (q - quit): ")
        if answer is None or answer.lower() == "q":
            return None
        if answer == "":
            value = current or default
            if not value and not allow_empty:
                print(red("A value is required."))
                continue
        elif answer.isdigit() and 1 <= int(answer) <= len(choices):
            value = choices[int(answer) - 1][0]
        elif allow_custom:
            value = answer
        else:
            print(red("Enter a number from the list."))
            continue
        if validator:
            err = validator(value)
            if err:
                print(red(f"{err}"))
                continue
        return value


def configure(s, vs_list):
    """Walk through every option. Returns False if the user aborted (EOF)."""
    print(bold("\n=== Setup ==="))

    # Qt
    kits = detect_qt_kits()
    choices = list(kits)
    if s["qt"] and s["qt"] not in [k for k, _ in kits]:
        choices.insert(0, (s["qt"], "current"))

    def qt_validator(x):
        kit = qt_kit_from_path(x)
        ok, note = validate_qt(kit or x)
        return "" if ok else note

    v = choose("Qt directory (e.g. C:/Qt/6.8.0/msvc2022_64)", s["qt"], choices,
               default=kits[0][0] if kits else "", validator=qt_validator)
    if v is None:
        return False
    s["qt"] = qt_kit_from_path(v) or norm_path(v)
    save_settings(s)

    # generator
    choices = [(vs["generator"], vs["path"]) for vs in vs_list]
    for g in NINJA_GENERATORS:
        choices.append((g, "ninja found" if shutil.which("ninja") else "requires ninja on PATH"))

    def gen_validator(x):
        if x in NINJA_GENERATORS and not shutil.which("ninja"):
            return "ninja not found on PATH"
        return ""

    v = choose("CMake generator", s["generator"], choices,
               default=vs_list[0]["generator"] if vs_list else "", validator=gen_validator)
    if v is None:
        return False
    s["generator"] = v
    match = next((vs for vs in vs_list if vs["generator"] == v), None)
    s["vs_instance"] = (match["instance"] or "") if match else ""
    save_settings(s)

    if is_vs_generator(s["generator"]):
        if match is None:
            v = choose('VS instance (CMAKE_GENERATOR_INSTANCE, "<path>,version=<ver>")', s["vs_instance"],
                       [("", "let CMake choose")], allow_empty=True)
            if v is None:
                return False
            s["vs_instance"] = v
            save_settings(s)
        v = choose("architecture", s["arch"], [(a, "") for a in ARCHS],
                   default=DEFAULT_ARCH, allow_custom=False)
        if v is None:
            return False
        s["arch"] = v
        save_settings(s)

    # version
    git_ver = detect_git_version()
    choices = []
    if git_ver:
        choices.append((git_ver, "latest git tag"))
    choices.append((DEFAULT_VERSION, "development default"))
    v = choose("version (X.Y.Z)", s["version"], choices, default=git_ver or DEFAULT_VERSION,
               validator=lambda x: "" if re.fullmatch(r"\d+\.\d+\.\d+", x) else "expected X.Y.Z")
    if v is None:
        return False
    s["version"] = v
    save_settings(s)

    # directories
    def dir_validator(x):
        if not Path(x).expanduser().is_absolute():
            return "must be an absolute path"
        if Path(x).expanduser().resolve() == SCRIPT_DIR.resolve():
            return "must not be the source directory"
        return ""

    for key, label, default in (("ide_dir", "IDE directory (CMake build tree, .sln)", ROOT_DIR / "_ide"),
                                ("build_dir", "build directory (exe + runtime resources)", ROOT_DIR / "_build"),
                                ("install_dir", "install directory (cmake --install)", ROOT_DIR / "_install")):
        choices = [(default.as_posix(), "default")]
        if s[key] and norm_path(s[key]) != default.as_posix():
            choices.insert(0, (norm_path(s[key]), "current"))
        v = choose(label, s[key], choices, default=default.as_posix(), validator=dir_validator)
        if v is None:
            return False
        s[key] = norm_path(v)
        save_settings(s)

    return True


# ---------------------------------------------------------------------------
# CMake
# ---------------------------------------------------------------------------
def build_command(s):
    major, minor, patch = (str(int(x)) for x in s["version"].split("."))
    cmd = ["cmake", "-G", s["generator"]]
    if is_vs_generator(s["generator"]):
        cmd += ["-A", s["arch"]]
        if s["vs_instance"]:
            cmd += [f"-DCMAKE_GENERATOR_INSTANCE={s['vs_instance']}"]
    cmd += [
        f"-DCMAKE_PREFIX_PATH={Path(s['qt']).as_posix()}",
        f"-DBUILD_MAJOR_VERSION={major}",
        f"-DBUILD_MINOR_VERSION={minor}",
        f"-DBUILD_PATCH_VERSION={patch}",
        f"-DBUILD_DIR={Path(s['build_dir']).as_posix()}",
        f"-DINSTALL_DIR={Path(s['install_dir']).as_posix()}",
        "-S", SCRIPT_DIR.as_posix(),
        "-B", Path(s["ide_dir"]).as_posix(),
    ]
    return cmd, f"{major}.{minor}.{patch}"


def format_command(cmd):
    return " ".join(f'"{c}"' if " " in c else c for c in cmd)


def run_cmake(s, dry_run=False):
    cmd, normalized = build_command(s)
    if normalized != s["version"]:
        print(yellow(f"Note: version {s['version']} is normalized to {normalized} for the source code "
                     "(leading zeros would be read as octal by C++)."))
    print(bold("Command:"))
    print("  " + format_command(cmd))
    print()
    if dry_run:
        return 0

    for key in ("ide_dir", "build_dir", "install_dir"):
        Path(s[key]).mkdir(parents=True, exist_ok=True)

    result = subprocess.run(cmd)
    print()
    if result.returncode != 0:
        print(red(f"cmake failed (exit code {result.returncode})"))
        return result.returncode

    print(green("cmake finished successfully."))
    if is_vs_generator(s["generator"]):
        slns = sorted(Path(s["ide_dir"]).glob("*.sln"))
        if slns:
            print(f"Solution: {slns[0]}")
    return 0


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def parse_args():
    p = argparse.ArgumentParser(description="Generate the CMake project for LMS.",
                                formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    p.add_argument("--qt", metavar="DIR", help="Qt kit directory, e.g. C:/Qt/6.8.0/msvc2022_64")
    p.add_argument("--generator", "-G", metavar="NAME", help="CMake generator")
    p.add_argument("--vs-instance", metavar="PATH[,version=X]", help="CMAKE_GENERATOR_INSTANCE")
    p.add_argument("--arch", "-A", metavar="ARCH", help=f"Architecture for Visual Studio ({'/'.join(ARCHS)})")
    p.add_argument("--version", metavar="X.Y.Z", help="Build version")
    p.add_argument("--ide-dir", metavar="DIR", help="CMake build tree (.sln)")
    p.add_argument("--build-dir", metavar="DIR", help="exe and runtime resources")
    p.add_argument("--install-dir", metavar="DIR", help="cmake --install target")
    p.add_argument("--reset", action="store_true", help="ignore saved settings")
    p.add_argument("--yes", "-y", action="store_true", help="non-interactive: run cmake if all options are valid")
    p.add_argument("--dry-run", action="store_true", help="non-interactive: print the cmake command only")
    p.add_argument("--no-color", action="store_true", help="disable colored output")
    return p.parse_args()


def apply_args(s, args):
    if args.qt:
        s["qt"] = qt_kit_from_path(args.qt) or norm_path(args.qt)
    if args.generator:
        s["generator"] = args.generator
        s["vs_instance"] = args.vs_instance or ""
    elif args.vs_instance:
        s["vs_instance"] = args.vs_instance
    if args.arch:
        s["arch"] = args.arch
    if args.version:
        s["version"] = args.version
    for key, val in (("ide_dir", args.ide_dir), ("build_dir", args.build_dir), ("install_dir", args.install_dir)):
        if val:
            s[key] = norm_path(val)


def menu():
    print(bold("What next?"))
    print("  1) Run CMake" + dim("  (default)"))
    print("  2) Reconfigure")
    print("  3) Quit")
    answer = ask("Select [1-3] (default 1, q - quit): ")
    if answer is None or answer.lower() == "q":
        return "3"
    return answer or "1"


def main():
    args = parse_args()
    Color.setup(force_off=args.no_color)

    if not (SCRIPT_DIR / "CMakeLists.txt").exists():
        print(red(f"CMakeLists.txt not found in {SCRIPT_DIR}"), file=sys.stderr)
        return 2

    vs_list = detect_visual_studios()

    s = default_settings()
    if not args.reset:
        s.update(load_settings())
    if not s["qt"]:
        kits = detect_qt_kits()
        if kits:
            s["qt"] = kits[0][0]
    if not s["generator"] and vs_list:
        s["generator"] = vs_list[0]["generator"]
        s["vs_instance"] = vs_list[0]["instance"] or ""
    apply_args(s, args)

    print(bold("LMS - CMake generator"))

    # Non-interactive -------------------------------------------------------
    if args.yes or args.dry_run:
        rows = validate(s, vs_list)
        print_table(rows)
        bad = [r for r in rows if not r.ok]
        if bad:
            print(red("Invalid options: " + "; ".join(f"{r.label}: {r.note}" for r in bad)), file=sys.stderr)
            return 2
        save_settings(s)
        return run_cmake(s, dry_run=args.dry_run)

    # Interactive -----------------------------------------------------------
    while True:
        rows = validate(s, vs_list)
        print_table(rows)

        bad = [r for r in rows if not r.ok]
        if bad:
            # Not enough information: go straight to setup, no menu.
            print(red("Missing or invalid: " + ", ".join(r.label for r in bad)))
            if not configure(s, vs_list):
                print("Quit.")
                return 0
            save_settings(s)
            print(dim(f"\nSettings saved to {SETTINGS_FILE}"))
            continue

        save_settings(s)
        choice = menu()
        if choice == "1":
            save_settings(s)
            print()
            return run_cmake(s)
        elif choice == "2":
            if not configure(s, vs_list):
                print("Quit.")
                return 0
            save_settings(s)
            print(dim(f"\nSettings saved to {SETTINGS_FILE}"))
        elif choice == "3":
            return 0
        else:
            print(red("Please enter 1, 2, 3 or q."))


if __name__ == "__main__":
    sys.exit(main())
