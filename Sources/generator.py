#!/usr/bin/env python3
"""
Interactive CMake project generator for LMS (Windows and Linux/macOS).

Run without arguments for the interactive mode:

    python generator.py

    1. A table of CMake options is shown. Valid values are green, invalid red.
    2. If everything is green the menu offers: Reconfigure / Run CMake / Quit
       (default: Run CMake). If something is red the default is Reconfigure.
    3. Reconfigure walks through every option, shows the available choices
       as a numbered list and asks for a value. At the end the menu is shown
       again.

Settings are remembered in <root>/.generator.json.

Platforms:
    Windows   Visual Studio generators (auto-detected) or Ninja.
    Linux/mac Ninja, Ninja Multi-Config or Unix Makefiles. Qt is looked up in
              ~/Qt, /opt/Qt, the system install (/usr/lib/*/cmake/Qt6) and
              through qtpaths6 / qmake6 on PATH.

Output directories:
    The IDE, build and install directories are base directories. The script
    appends a target subdirectory "<os>-<arch>" (windows-x64, linux-x64,
    linux-ARM64, ...) so one checkout can be configured for several targets
    side by side:

        <root>/_ide/windows-x64      <root>/_ide/linux-x64
        <root>/_build/windows-x64    <root>/_build/linux-x64

Non-interactive use (CI): pass --yes. The table is printed, CMake runs if every
option is valid, otherwise the script exits with code 2.

    python generator.py --qt C:/Qt/6.8.0/msvc2022_64 --version 25.1.2 --yes
    python generator.py --qt ~/Qt/6.8.0/gcc_64 -G Ninja --version 25.1.2 --yes
"""

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent   # .../LMS/Sources  (root CMakeLists.txt)
ROOT_DIR = SCRIPT_DIR.parent                   # .../LMS
SETTINGS_FILE = ROOT_DIR / ".generator.json"

DEFAULT_VERSION = "99.99.99"

# ---------------------------------------------------------------------------
# Host platform
# ---------------------------------------------------------------------------
if os.name == "nt":
    HOST_OS = "windows"
elif sys.platform == "darwin":
    HOST_OS = "macos"
else:
    HOST_OS = "linux"
IS_WINDOWS = HOST_OS == "windows"


def detect_host_arch():
    m = platform.machine().lower()
    if m in ("amd64", "x86_64"):
        return "x64"
    if m in ("arm64", "aarch64"):
        return "ARM64"
    if m in ("x86", "i386", "i686"):
        return "Win32" if IS_WINDOWS else "x86"
    return m or "unknown"


HOST_ARCH = detect_host_arch()
DEFAULT_ARCH = HOST_ARCH

QT_MODULES = ("Core", "Gui", "Widgets", "Network", "SerialPort", "Sql", "Concurrent")
if IS_WINDOWS:
    QT_ROOTS = ("C:/Qt", "D:/Qt", "E:/Qt")                       # official layout: <root>/<version>/<kit>
    QT_THIRDPARTY_ROOTS = ("C:/_ThirdParty/libqt", "D:/_ThirdParty/libqt", "E:/_ThirdParty/libqt")  # team layout: <root>/<os>/<arch>
    QT_SYSTEM_CMAKE_GLOBS = ()
else:
    QT_ROOTS = ("~/Qt", "/opt/Qt", "/opt/qt", "/usr/local/Qt")
    QT_THIRDPARTY_ROOTS = ("~/_ThirdParty/libqt", "/opt/_ThirdParty/libqt")
    QT_SYSTEM_CMAKE_GLOBS = (                                    # distro packages: directory holding Qt6Config.cmake
        "/usr/lib/*/cmake/Qt6",
        "/usr/lib64/cmake/Qt6",
        "/usr/lib/cmake/Qt6",
        "/usr/local/lib/cmake/Qt6",
        "/usr/local/lib/*/cmake/Qt6",
    )

VS_YEAR_TO_MAJOR = {"2017": 15, "2019": 16, "2022": 17, "2026": 18}
VS_MAJOR_TO_YEAR = {v: k for k, v in VS_YEAR_TO_MAJOR.items()}
VS_EDITIONS = ("Enterprise", "Professional", "Community", "Preview", "BuildTools")
NINJA_GENERATORS = ("Ninja", "Ninja Multi-Config")
MAKE_GENERATORS = ("Unix Makefiles",)
MULTI_CONFIG_GENERATORS = ("Ninja Multi-Config",)
VS_ARCHS = ("x64", "Win32", "ARM64")


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
        if cls.enabled and IS_WINDOWS:
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


def is_multi_config_generator(name):
    return is_vs_generator(name) or name in MULTI_CONFIG_GENERATORS


# ---------------------------------------------------------------------------
# Qt
# A Qt value may be a kit directory (<kit>/lib/cmake/Qt6/Qt6Config.cmake, the
# official installer and the team layout), the directory that holds
# Qt6Config.cmake itself (distro packages, e.g. /usr/lib/x86_64-linux-gnu/cmake/Qt6)
# or any path inside one of those.
# ---------------------------------------------------------------------------
def qt_cmake_dir(value):
    """Directory holding Qt6Config.cmake for `value`, or '' if none."""
    if not value:
        return ""
    p = Path(value).expanduser()
    candidates = [p / "lib" / "cmake" / "Qt6", p / "cmake" / "Qt6", p]
    candidates += list(p.parents)[:3]
    for c in candidates:
        if (c / "Qt6Config.cmake").is_file():
            return c.resolve().as_posix()
    return ""


def qt_kit_dir(cmake_dir):
    """Kit root for a <kit>/lib/cmake/Qt6 layout, '' for a system install."""
    if not cmake_dir:
        return ""
    kit = Path(cmake_dir).parent.parent.parent
    if (kit / "lib" / "cmake" / "Qt6" / "Qt6Config.cmake").is_file():
        return kit.resolve().as_posix()
    return ""


def qt_normalize(value):
    """Canonical Qt value: kit dir when it is a kit, else the Qt6 cmake dir. '' if invalid."""
    cmake_dir = qt_cmake_dir(value)
    return qt_kit_dir(cmake_dir) or cmake_dir


def qt_missing_modules(value):
    cmake_dir = qt_cmake_dir(value)
    if not cmake_dir:
        return list(QT_MODULES)
    base = Path(cmake_dir).parent
    return [m for m in QT_MODULES if not (base / f"Qt6{m}").is_dir()]


def detect_qt_kits():
    """Return list of (qt_value, description)."""
    found = []

    def add(path, desc):
        qt = qt_normalize(path)
        if qt and qt not in [k for k, _ in found]:
            found.append((qt, desc))

    for var in ("Qt6_DIR", "QT_DIR", "QTDIR", "CMAKE_PREFIX_PATH"):
        for p in os.environ.get(var, "").split(os.pathsep):
            if p:
                add(p, var)

    for root in QT_ROOTS:
        root_path = Path(root).expanduser()
        if not root_path.is_dir():
            continue
        versions = sorted((d for d in root_path.glob("6.*") if d.is_dir()),
                          key=lambda d: [int(x) if x.isdigit() else 0 for x in d.name.split(".")],
                          reverse=True)
        for ver in versions:
            for kit in sorted(ver.iterdir()):
                if kit.is_dir() and qt_cmake_dir(kit):
                    add(kit, f"Qt {ver.name}")

    for root in QT_THIRDPARTY_ROOTS:
        root_path = Path(root).expanduser()
        if not root_path.is_dir():
            continue
        for kit in sorted(root_path.glob("*/*")):
            if kit.is_dir() and qt_cmake_dir(kit):
                add(kit, "third-party")

    for pattern in QT_SYSTEM_CMAKE_GLOBS:
        for d in sorted(Path("/").glob(pattern.lstrip("/"))):
            add(d, "system")

    for tool in ("qtpaths6", "qmake6", "qtpaths", "qmake"):
        exe = shutil.which(tool)
        if not exe:
            continue
        libs = run_capture([exe, "-query", "QT_INSTALL_LIBS"])
        if libs:
            add(Path(libs) / "cmake" / "Qt6", tool)
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
    Return a list of dicts {generator, instance, path}. Empty outside Windows.
    `instance` is set only when the installation is not registered with the
    Visual Studio Installer; CMake then needs CMAKE_GENERATOR_INSTANCE.
    """
    if not IS_WINDOWS:
        return []
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
# Generators
# ---------------------------------------------------------------------------
def generator_choices(vs_list):
    choices = []
    if IS_WINDOWS:
        choices += [(vs["generator"], vs["path"]) for vs in vs_list]
    for g in NINJA_GENERATORS:
        choices.append((g, "ninja found" if shutil.which("ninja") else "requires ninja on PATH"))
    if not IS_WINDOWS:
        for g in MAKE_GENERATORS:
            choices.append((g, "make found" if shutil.which("make") else "requires make on PATH"))
    return choices


def default_generator(vs_list):
    if IS_WINDOWS and vs_list:
        return vs_list[0]["generator"]
    if shutil.which("ninja"):
        return "Ninja"
    if not IS_WINDOWS and shutil.which("make"):
        return "Unix Makefiles"
    return ""


def generator_error(name, vs_instance, vs_list):
    """'' when `name` can be used on this host, else the reason."""
    if not name:
        return "not set"
    if is_vs_generator(name):
        if not IS_WINDOWS:
            return f"Visual Studio generators are only available on Windows (host: {HOST_OS})"
        if any(v["generator"] == name for v in vs_list) or vs_instance:
            return ""
        return "Visual Studio not detected (set VS instance)"
    if name in NINJA_GENERATORS and not shutil.which("ninja"):
        return "ninja not found on PATH"
    if name in MAKE_GENERATORS and not shutil.which("make"):
        return "make not found on PATH"
    return ""


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
# Target (os-arch) and output directories
# ---------------------------------------------------------------------------
def effective_arch(s):
    """Visual Studio builds the architecture given by -A; every other generator builds for the host."""
    return s["arch"] if is_vs_generator(s["generator"]) else HOST_ARCH


def target_tag(s):
    return f"{HOST_OS}-{effective_arch(s)}"


def effective_dir(s, key):
    """<base dir>/<os>-<arch>"""
    return (Path(s[key]) / target_tag(s)).as_posix()


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------
class Row:
    def __init__(self, key, label, value, ok, note=""):
        self.key, self.label, self.value, self.ok, self.note = key, label, value, ok, note


def validate_qt(value):
    if not value:
        return False, "not set"
    path = Path(value).expanduser()
    if not path.exists():
        return False, f"directory does not exist: {value}"
    if not path.is_dir():
        return False, f"not a directory: {value}"
    if not qt_cmake_dir(path):
        return False, "Qt6Config.cmake not found (expected <kit>/lib/cmake/Qt6 or a Qt6 cmake directory)"
    missing = qt_missing_modules(value)
    if missing:
        return False, "missing Qt modules: " + ", ".join(missing)
    return True, ""


def validate(s, vs_list):
    rows = []

    qt_ok, note = validate_qt(s["qt"])
    rows.append(Row("qt", "Qt dir", s["qt"], qt_ok, note))

    gen = s["generator"]
    note = generator_error(gen, s["vs_instance"], vs_list)
    rows.append(Row("generator", "CMake generator", gen, not note, note))

    if is_vs_generator(gen):
        rows.append(Row("vs_instance", "VS instance", s["vs_instance"] or "(CMake default)", True))
        arch_ok = s["arch"] in VS_ARCHS
        rows.append(Row("arch", "Architecture", s["arch"], arch_ok, "" if arch_ok else f"one of {', '.join(VS_ARCHS)}"))

    rows.append(Row("target", "Target", target_tag(s), True))

    ver_ok = bool(re.fullmatch(r"\d+\.\d+\.\d+", s["version"]))
    rows.append(Row("version", "Version", s["version"], ver_ok, "" if ver_ok else "expected X.Y.Z"))

    for key, label in (("ide_dir", "IDE dir"), ("build_dir", "Build dir"), ("install_dir", "Install dir")):
        v = s[key]
        if not v:
            ok, note, shown = False, "not set", ""
        elif not Path(v).is_absolute():
            ok, note, shown = False, "must be an absolute path", v
        elif Path(v).resolve() == SCRIPT_DIR.resolve():
            ok, note, shown = False, "must not be the source directory", v
        else:
            ok, note, shown = True, "", effective_dir(s, key)
        rows.append(Row(key, label, shown, ok, note))

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
    print(bold(f"\n=== Setup ({HOST_OS}, host arch {HOST_ARCH}) ==="))

    # Qt
    kits = detect_qt_kits()
    choices = list(kits)
    if s["qt"] and s["qt"] not in [k for k, _ in kits]:
        choices.insert(0, (s["qt"], "current"))

    def qt_validator(x):
        ok, note = validate_qt(qt_normalize(x) or x)
        return "" if ok else note

    if IS_WINDOWS:
        example = "C:/Qt/6.8.0/msvc2022_64"
    else:
        example = "~/Qt/6.8.0/gcc_64 or /usr/lib/x86_64-linux-gnu/cmake/Qt6"
    v = choose(f"Qt directory (e.g. {example})", s["qt"], choices,
               default=kits[0][0] if kits else "", validator=qt_validator)
    if v is None:
        return False
    s["qt"] = qt_normalize(v) or norm_path(v)
    save_settings(s)

    # generator
    choices = generator_choices(vs_list)
    if s["generator"] and s["generator"] not in [c for c, _ in choices]:
        choices.insert(0, (s["generator"], "current"))

    def gen_validator(x):
        # vs_instance is unknown until the generator is chosen; an undetected VS is handled below.
        err = generator_error(x, s["vs_instance"], vs_list)
        return "" if not err or err.startswith("Visual Studio not detected") else err

    v = choose("CMake generator", s["generator"], choices,
               default=default_generator(vs_list), validator=gen_validator)
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
        v = choose("architecture", s["arch"] if s["arch"] in VS_ARCHS else "", [(a, "") for a in VS_ARCHS],
                   default=HOST_ARCH if HOST_ARCH in VS_ARCHS else "x64", allow_custom=False)
        if v is None:
            return False
        s["arch"] = v
        save_settings(s)
    else:
        s["arch"] = HOST_ARCH
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

    tag = target_tag(s)
    for key, label, default in (("ide_dir", "IDE base directory (CMake build tree, .sln)", ROOT_DIR / "_ide"),
                                ("build_dir", "build base directory (exe + runtime resources)", ROOT_DIR / "_build"),
                                ("install_dir", "install base directory (cmake --install)", ROOT_DIR / "_install")):
        choices = [(default.as_posix(), "default")]
        if s[key] and norm_path(s[key]) != default.as_posix():
            choices.insert(0, (norm_path(s[key]), "current"))
        v = choose(f"{label}; the target subdirectory /{tag} is appended", s[key], choices,
                   default=default.as_posix(), validator=dir_validator)
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

    # Kit layout: CMAKE_PREFIX_PATH=<kit> (also finds tools next to it).
    # Any layout: Qt6_DIR=<dir holding Qt6Config.cmake>, which is what distro packages need.
    qt_cmake = qt_cmake_dir(s["qt"])
    qt_kit = qt_kit_dir(qt_cmake)
    if qt_kit:
        cmd += [f"-DCMAKE_PREFIX_PATH={qt_kit}"]
    if qt_cmake:
        cmd += [f"-DQt6_DIR={qt_cmake}"]

    cmd += [
        f"-DBUILD_MAJOR_VERSION={major}",
        f"-DBUILD_MINOR_VERSION={minor}",
        f"-DBUILD_PATCH_VERSION={patch}",
        f"-DBUILD_DIR={effective_dir(s, 'build_dir')}",
        f"-DINSTALL_DIR={effective_dir(s, 'install_dir')}",
        "-S", SCRIPT_DIR.as_posix(),
        "-B", effective_dir(s, "ide_dir"),
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
        Path(effective_dir(s, key)).mkdir(parents=True, exist_ok=True)

    result = subprocess.run(cmd)
    print()
    if result.returncode != 0:
        print(red(f"cmake failed (exit code {result.returncode})"))
        return result.returncode

    print(green("cmake finished successfully."))
    ide_dir = effective_dir(s, "ide_dir")
    if is_vs_generator(s["generator"]):
        slns = sorted(Path(ide_dir).glob("*.sln"))
        if slns:
            print(f"Solution: {slns[0]}")
    elif is_multi_config_generator(s["generator"]):
        print(f"Build:    cmake --build {ide_dir} --config RelWithDebInfo")
    else:
        print(f"Build:    cmake --build {ide_dir}")
    return 0


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def parse_args():
    p = argparse.ArgumentParser(description="Generate the CMake project for LMS.",
                                formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    p.add_argument("--qt", metavar="DIR", help="Qt kit directory (C:/Qt/6.8.0/msvc2022_64, ~/Qt/6.8.0/gcc_64) "
                                               "or the directory holding Qt6Config.cmake")
    p.add_argument("--generator", "-G", metavar="NAME", help="CMake generator")
    p.add_argument("--vs-instance", metavar="PATH[,version=X]", help="CMAKE_GENERATOR_INSTANCE (Windows)")
    p.add_argument("--arch", "-A", metavar="ARCH", help=f"Architecture for Visual Studio ({'/'.join(VS_ARCHS)})")
    p.add_argument("--version", metavar="X.Y.Z", help="Build version")
    p.add_argument("--ide-dir", metavar="DIR", help="CMake build tree base (.sln); <os>-<arch> is appended")
    p.add_argument("--build-dir", metavar="DIR", help="exe and runtime resources base; <os>-<arch> is appended")
    p.add_argument("--install-dir", metavar="DIR", help="cmake --install target base; <os>-<arch> is appended")
    p.add_argument("--reset", action="store_true", help="ignore saved settings")
    p.add_argument("--yes", "-y", action="store_true", help="non-interactive: run cmake if all options are valid")
    p.add_argument("--dry-run", action="store_true", help="non-interactive: print the cmake command only")
    p.add_argument("--no-color", action="store_true", help="disable colored output")
    return p.parse_args()


def apply_args(s, args):
    if args.qt:
        s["qt"] = qt_normalize(args.qt) or norm_path(args.qt)
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


def drop_foreign_host_settings(s, vs_list):
    """
    .generator.json may come from a checkout last configured on another OS
    (e.g. a Windows "E:/LMS/_ide" path or a Visual Studio generator loaded on
    Linux). Such values can never be valid here, so fall back to the host
    defaults instead of forcing a reconfigure for each of them.
    """
    defaults = default_settings()
    if is_vs_generator(s["generator"]) and not IS_WINDOWS:
        s["generator"] = default_generator(vs_list)
        s["vs_instance"] = ""
    for key in ("ide_dir", "build_dir", "install_dir"):
        if s[key] and not Path(s[key]).is_absolute():      # drive-letter path on POSIX, or POSIX path on Windows
            s[key] = defaults[key]
    if s["qt"] and not Path(s["qt"]).is_absolute():
        s["qt"] = ""


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
        drop_foreign_host_settings(s, vs_list)
    if not s["qt"]:
        kits = detect_qt_kits()
        if kits:
            s["qt"] = kits[0][0]
    if not s["generator"]:
        s["generator"] = default_generator(vs_list)
        match = next((vs for vs in vs_list if vs["generator"] == s["generator"]), None)
        s["vs_instance"] = (match["instance"] or "") if match else ""
    if not is_vs_generator(s["generator"]):
        s["arch"] = HOST_ARCH
    apply_args(s, args)

    print(bold(f"LMS - CMake generator ({HOST_OS}, {HOST_ARCH})"))

    # Non-interactive -------------------------------------------------------
    if args.yes or args.dry_run:
        rows = validate(s, vs_list)
        print_table(rows)
        bad = [r for r in rows if not r.ok]
        if bad:
            sys.stdout.flush()
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
