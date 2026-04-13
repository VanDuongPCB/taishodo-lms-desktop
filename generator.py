import os
import subprocess


def main():
    cwd = os.getcwd()
    exec_dir = os.path.dirname(os.path.abspath(__file__))

    vcpkg_dir = os.path.join( exec_dir, "_vcpkg" )
    cmake_toolchain_file = os.path.join( vcpkg_dir, "scripts", "buildsystems", "vcpkg.cmake" )
    ide_dir = os.path.join( cwd, "_ide" )
    build_dir = os.path.join( cwd, "_build" )
    install_dir = os.path.join( cwd, "_install" )
    source_dir = exec_dir

    command = []

    # Cmake
    command.append("cmake")

    # Visual Studio
    command.extend(["-G", "Visual Studio 18 2026"])

    # Architecture
    command.extend(["-A", "x64"])

    # Vcpkg
    command.append(f"-DCMAKE_TOOLCHAIN_FILE={cmake_toolchain_file}")

    # Version
    command.append("-DBUILD_MAJOR_VERSION=99")
    command.append("-DBUILD_MINOR_VERSION=99")
    command.append("-DBUILD_PATCH_VERSION=99")

    # Outputs
    command.append(f"-DIDE_DIR={ide_dir}")
    command.append(f"-DBUILD_DIR={build_dir}")
    command.append(f"-DINSTALL_DIR={install_dir}")

    # Source
    command.extend(["-S", source_dir])

    # Create directories
    os.makedirs( ide_dir, exist_ok = True )
    os.makedirs( build_dir, exist_ok = True )
    os.makedirs( install_dir, exist_ok = True )

    # Run
    subprocess.run(command, cwd=ide_dir, shell=True)
    pass

if __name__ == "__main__":
    main()