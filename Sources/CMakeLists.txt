cmake_minimum_required(VERSION 3.10)
project(Desktop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES *.cpp)
file(GLOB UI_FILES *.ui)
file(GLOB RESOURCE_FILES ../Assets/Icons/*.qrc)
file(GLOB RESOURCE_ICONS ../Assets/Icons/*.rc)

source_group("Forms" FILES ${UI_FILES})
source_group("Resources" FILES ${RESOURCE_FILES})
source_group("Resources" FILES ${RESOURCE_ICONS})

set(APP_TYPE WIN32)
add_executable(${PROJECT_NAME} ${APP_TYPE} ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES} ${RESOURCE_FILES} ${RESOURCE_ICONS})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network SerialPort Sql Concurrent)

target_link_libraries(${PROJECT_NAME} PRIVATE
Qt6::Core
Qt6::Widgets
Qt6::Network
Qt6::SerialPort
Qt6::Sql
Qt6::Concurrent
)

# Qt deploy
get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

# Post-build
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                $<$<CONFIG:Debug>:--debug>
                $<$<CONFIG:Release>:--release>
                $<$<CONFIG:RelWithDebInfo>:--release>
                $<$<CONFIG:MinSizeRel>:--release>
                --compiler-runtime
                "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Running windeployqt..."
    )
    file(COPY "../Default Files" DESTINATION ${BUILD_DIR})
    file(COPY "../Assets/Themes" DESTINATION ${BUILD_DIR})
endif() 



