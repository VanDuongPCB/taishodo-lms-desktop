import os
import re
from pathlib import Path
import subprocess


def check_qt():
    print("Checking for Qt installation...")
    current_dir = os.path.dirname(os.path.abspath(__file__))
    third_party_dir = os.path.join(current_dir, "_3rdparty")
    if os.path.exists(third_party_dir) == False:
        print("Third party directory not found.")
        return False
    
    os_target = "Windows"

    print(f"Looking in: {third_party_dir}")

    pattern = re.compile(r"^libqt_\d+\.\d+\.\d+$")
    dir = Path(third_party_dir)
    for p in dir.iterdir():
        if p.is_dir():
            m = pattern.match(p.name)
            if m:
                version_numbers = tuple(map(int, p.name.split('_')[1].split('.')))
                if version_numbers and len(version_numbers) == 3:
                    print(f"Found Qt:")
                    print(f"Setting QT_DIR to: {third_party_dir}/{p.name}/{os_target}/lib/cmake/Qt{version_numbers[0]}")
                    print(f"Setting QT_VERSION to: {version_numbers[0]}.{version_numbers[1]}.{version_numbers[2]}")
                    
                    os.environ.update({
                        f"QT_DIR": f"{third_party_dir}/{p.name}/{os_target}",
                        f"QT{version_numbers[0]}_DIR": f"{third_party_dir}/{p.name}/{os_target}/lib/cmake/Qt{version_numbers[0]}",
                        "QT_VERSION_MAJOR": str(version_numbers[0]),
                        "QT_VERSION_MINOR": str(version_numbers[1]),
                        "QT_VERSION_PATCH": str(version_numbers[2]),
                        "QT_VERSION_STR": f"{version_numbers[0]}.{version_numbers[1]}.{version_numbers[2]}",
                        "QT_VERSION": f"{version_numbers[0]}.{version_numbers[1]}.{version_numbers[2]}",
                    })

                    print("Qt configuration complete.")
                    return True

    print("No valid Qt installation found.")
    return False
    pass

def check_sources():
    print("Checking source directory...")
    current_dir = os.path.dirname(os.path.abspath(__file__))
    source_dir = os.path.join(current_dir, "Sources")
    if os.path.exists(source_dir) == False:
        print("Source directory not found.")
        return False
    
    cmake_file = os.path.join(source_dir, "CMakeLists.txt")
    if os.path.exists(cmake_file) == False:
        print("CMakeLists.txt not found in source directory.")
        return False


    os.environ.update({
                        "SOURCE_DIR": f"{source_dir}",
                    })
    print("Source directory found.")
    return True
    pass


def generate_dirs():
    print("Generating directory structure...")
    current_dir = os.path.dirname(os.path.abspath(__file__))
    ide_dir = os.path.join(current_dir, "_ide")
    build_dir = os.path.join(current_dir, "_build")

    os.environ.update({
                        "IDE_DIR": f"{ide_dir}",
                        "BUILD_DIR": f"{build_dir}",
                    })
    
    os.makedirs(ide_dir, exist_ok=True)
    os.makedirs(build_dir, exist_ok=True)
    return True

def check_version():
    print("Checking version...")

    major_vesion = 99
    minor_version = 9
    patch_version = 9

    os.environ.update({
                        "BUILD_MAJOR_VERSION": f"{major_vesion}",
                        "BUILD_MINOR_VERSION": f"{minor_version}",
                        "BUILD_PATCH_VERSION": f"{patch_version}",
                        "BUILD_VERSION": f"{major_vesion}.{minor_version}.{patch_version}",
                    })
    # Placeholder for version checking logic
    return True

def generate_ide_files():
    print("Generating IDE files...")
    print(f"IDE_DIR = {os.environ['IDE_DIR']}")
    print(f"BUILD_DIR = {os.environ['BUILD_DIR']}")
    print(f"SOURCE_DIR = {os.environ['SOURCE_DIR']}")
    print(f"QT_DIR = {os.environ['QT_DIR']}")
    print(f"QT_VERSION = {os.environ['QT_VERSION']}")
    print(f"QT_VERSION_MAJOR = {os.environ['QT_VERSION_MAJOR']}")
    print(f"QT_VERSION_MINOR = {os.environ['QT_VERSION_MINOR']}")
    print(f"QT_VERSION_PATCH = {os.environ['QT_VERSION_PATCH']}")
    print(f"BUILD_MAJOR_VERSION = {os.environ['BUILD_MAJOR_VERSION']}")
    print(f"BUILD_MINOR_VERSION = {os.environ['BUILD_MINOR_VERSION']}")
    print(f"BUILD_PATCH_VERSION = {os.environ['BUILD_PATCH_VERSION']}")
    print(f"BUILD_VERSION = {os.environ['BUILD_VERSION']}")

    current_dir = os.path.dirname(os.path.abspath(__file__))
    cmake_dir = os.path.join(current_dir, "Sources")
    # Windows 
    subprocess.run(["cmake", cmake_dir], cwd=os.environ["IDE_DIR"])
    return True

def main():
    if not check_qt():
        return
    
    if not check_sources():
        return
    
    if not generate_dirs():
        return
    
    if not check_version():
        return

    generate_ide_files()
    pass


if __name__ == "__main__":
    main()