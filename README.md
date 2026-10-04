How to setup environment.

1. Prerequisites
    - Visual Studio 2022 or newer with the "Desktop development with C++" workload
    - CMake 3.21+ on PATH
    - Python 3.8+ on PATH
    - Qt 6.3+ (Qt Online Installer, https://www.qt.io/download-qt-installer)
        Select a Qt 6.x kit for MSVC 64-bit, e.g. "MSVC 2022 64-bit", plus the
        additional library "Qt Serial Port".
        (Core, Gui, Widgets, Network, Sql and Concurrent are part of the base kit.)

        Result: a kit directory such as C:\Qt\6.8.0\msvc2022_64 containing lib\cmake\Qt6.
        generator.py scans C:\Qt, D:\Qt, E:\Qt and the Qt6_DIR / QTDIR / CMAKE_PREFIX_PATH
        environment variables, so a default installation is found automatically.


2. Generate the Visual Studio solution
    python Sources\generator.py

    The script shows a table of the CMake options:
        green = valid, red = missing or invalid.

    If any option is red, Setup starts immediately. Otherwise a menu is shown:
        1) Run CMake          configure the project (default)
        2) Reconfigure        run Setup again
        3) Quit
        q also quits.

    Setup asks for each option in turn:

        Specify Qt directory (e.g. C:/Qt/6.8.0/msvc2022_64):

        Current: C:/Qt/6.8.0/msvc2022_64
        1) C:/Qt/6.10.1/msvc2022_64   Qt 6.10.1
        2) C:/Qt/6.8.0/msvc2022_64    Qt 6.8.0
        Enter (q - quit):

        - type a number to pick a listed choice,
        - type a value directly,
        - press Enter to keep the current value (or take the default),
        - type q to quit (answers accepted so far are kept).
        An invalid value is reported and the question is asked again.

    Options:
        Qt directory        Qt kit, must exist and contain lib/cmake/Qt6 with all required modules
        CMake generator     detected Visual Studio versions, Ninja, or any generator name
        VS instance         only asked when the chosen Visual Studio was not auto-detected
        Architecture        x64 / Win32 / ARM64 (Visual Studio generators only)
        Version             X.Y.Z, default is the latest git tag, else 99.99.99
        IDE directory       CMake build tree with LMS.sln (default: _ide)
        Build directory     LMS.exe plus "Default Files" and "Themes" (default: _build\<Config>)
        Install directory   target of cmake --install (default: _install)

    Settings are saved to .generator.json (ignored by git) as soon as they are
    valid: after each accepted Setup answer, whenever the table is all green,
    and before every cmake run (also with --yes / --dry-run).
    "Run CMake" prints the full cmake command, runs it, prints the path of the
    generated LMS.sln and exits (exit code = cmake's exit code).


3. Build
    Open _ide\LMS.sln in Visual Studio, or:
        cmake --build _ide --config RelWithDebInfo

    windeployqt runs after each build, so _build\RelWithDebInfo\ contains a runnable
    LMS.exe with the Qt DLLs, "Default Files" and "Themes" next to it.

    Flat folder for the installer (Install\*.iss):
        cmake --install _ide --config RelWithDebInfo


4. Non-interactive use (CI)
    python Sources\generator.py --qt C:\Qt\6.8.0\msvc2022_64 --version 25.1.2 --yes

    --yes        print the table and run cmake if every option is valid, else exit code 2
    --dry-run    print the table and the cmake command only
    --reset      ignore .generator.json
    --no-color   plain output
    --help       full list of flags (--qt, --generator, --arch, --vs-instance,
                 --version, --ide-dir, --build-dir, --install-dir)
