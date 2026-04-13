How to setup environment.

1. Install vcpkg
    Clone: 
        git clone https://github.com/microsoft/vcpkg.git _vcpkg

    Update:
        PS:     cd _vcpkg; .\bootstrap-vcpkg.bat
        CMD:    cd _vcpkg && bootstrap-vcpkg.bat
        Bash:   cd _vcpkg && ./bootstrap-vcpkg.sh

    Install:
        PS:     ./vcpkg.exe integrate install
        CMD:    .\vcpkg.exe integrate install
        Bash:   .\vcpkg.exe integrate install

    Install QT6:
        ./vcpkg.exe install qttools:x64-windows --recurse --clean-after-build
        ./vcpkg install qtserialport:x64-windows --clean-after-build


2. Run configuration
    Cd to root directory
    Run: python generator.py
    
