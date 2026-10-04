cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------------------
# Phiên bản
# generator.py truyền -DBUILD_MAJOR_VERSION / MINOR / PATCH.
# Không truyền thì mặc định 99.99.99 (bản dev).
# ---------------------------------------------------------------------------
set(BUILD_MAJOR_VERSION 99 CACHE STRING "Major version")
set(BUILD_MINOR_VERSION 99 CACHE STRING "Minor version")
set(BUILD_PATCH_VERSION 99 CACHE STRING "Patch version")

project(LMS
    VERSION ${BUILD_MAJOR_VERSION}.${BUILD_MINOR_VERSION}.${BUILD_PATCH_VERSION}
    DESCRIPTION "Laser Marking System"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Cấu hình chung
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt (bản cài chính thức hoặc vcpkg) dùng CRT động (/MD, /MDd). Khai báo tường minh để không lệch CRT.
# Thư mục Qt được generator.py truyền qua -DCMAKE_PREFIX_PATH=<kit>, ví dụ C:/Qt/6.8.0/msvc2022_64.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Cấu hình build. Mặc định là RelWithDebInfo ở mọi loại generator.
#  - Visual Studio / Ninja Multi-Config: danh sách cấu hình, cái đầu tiên là
#    cấu hình Visual Studio chọn sẵn khi mở solution. MinSizeRel bỏ vì không dùng.
#  - Ninja / Makefile (đơn cấu hình): CMAKE_BUILD_TYPE, chỉ đặt khi người dùng
#    chưa truyền -DCMAKE_BUILD_TYPE.
get_property(LMS_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(LMS_IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo;Release;Debug" CACHE STRING "Build configurations" FORCE)
    # Chỉ Ninja Multi-Config hiểu biến này; Visual Studio sẽ báo lỗi nếu đặt.
    if(CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
        set(CMAKE_DEFAULT_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Default configuration" FORCE)
    endif()
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS RelWithDebInfo Release Debug)
endif()

# ---------------------------------------------------------------------------
# Thư mục xuất
# generator.py truyền -DBUILD_DIR và -DINSTALL_DIR. Không truyền thì nằm trong
# thư mục build của CMake, không bao giờ rơi ra gốc ổ đĩa.
# ---------------------------------------------------------------------------
if(NOT DEFINED BUILD_DIR OR BUILD_DIR STREQUAL "")
    set(BUILD_DIR "${CMAKE_BINARY_DIR}/out")
endif()
set(LMS_OUTPUT_DIR "${BUILD_DIR}" CACHE PATH "Thư mục chứa exe và tài nguyên runtime")

if(DEFINED INSTALL_DIR AND NOT INSTALL_DIR STREQUAL "" AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" CACHE PATH "Thư mục install" FORCE)
endif()

# ---------------------------------------------------------------------------
# Thư mục con
# ---------------------------------------------------------------------------
add_subdirectory(Desktop)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LMS)
