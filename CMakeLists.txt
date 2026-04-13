cmake_minimum_required(VERSION 3.5)
project(LMS)

if (NOT DEFINED BUILD_MAJOR_VERSION)
    set( BUILD_MAJOR_VERSION 99 )
endif()

if (NOT DEFINED BUILD_MINOR_VERSION)
    set( BUILD_MINOR_VERSION 99 )
endif()

if (NOT DEFINED BUILD_PATCH_VERSION)
    set( BUILD_PATCH_VERSION 99 )
endif()

add_definitions(-DBUILD_MAJOR_VERSION=${BUILD_MAJOR_VERSION})
add_definitions(-DBUILD_MINOR_VERSION=${BUILD_MINOR_VERSION})
add_definitions(-DBUILD_PATCH_VERSION=${BUILD_PATCH_VERSION})

foreach(build_type ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${build_type} build_type_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${build_type_upper} "${BUILD_DIR}/${build_type}/exe")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${build_type_upper} "${BUILD_DIR}/${build_type}/libs")
endforeach()

add_subdirectory(Sources)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Desktop)