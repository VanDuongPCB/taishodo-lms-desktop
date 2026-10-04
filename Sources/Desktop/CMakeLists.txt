# Ứng dụng desktop LMS. Không khai báo project() ở đây: phiên bản, chuẩn C++
# và thư mục xuất đều lấy từ CMakeLists.txt gốc.

find_package(Qt6 6.3 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    SerialPort
    Sql
    Concurrent
)

# Bật AUTOMOC, AUTOUIC và các mặc định khuyến nghị của Qt 6.
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------------------
# Nguồn
# CONFIGURE_DEPENDS: thêm/xoá file thì CMake tự chạy lại, không cần configure tay.
# ---------------------------------------------------------------------------
set(LMS_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Assets")
set(LMS_DEFAULT_FILES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Default Files")

file(GLOB LMS_HEADERS   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB LMS_SOURCES   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB LMS_FORMS     CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
file(GLOB LMS_RESOURCES CONFIGURE_DEPENDS "${LMS_ASSETS_DIR}/Icons/*.qrc")

set(LMS_WIN_RESOURCES)
if(WIN32)
    file(GLOB LMS_WIN_RESOURCES CONFIGURE_DEPENDS "${LMS_ASSETS_DIR}/Icons/*.rc")
endif()

source_group("Header Files" FILES ${LMS_HEADERS})
source_group("Source Files" FILES ${LMS_SOURCES})
source_group("Forms"        FILES ${LMS_FORMS})
source_group("Resources"    FILES ${LMS_RESOURCES} ${LMS_WIN_RESOURCES})

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------
qt_add_executable(LMS
    ${LMS_SOURCES}
    ${LMS_HEADERS}
    ${LMS_FORMS}
    ${LMS_RESOURCES}
    ${LMS_WIN_RESOURCES}
)

set_target_properties(LMS PROPERTIES
    OUTPUT_NAME               "LMS"
    WIN32_EXECUTABLE          TRUE
    # Có $<CONFIG> trong đường dẫn nên generator đa cấu hình không tự thêm
    # thư mục con nữa. Exe và tài nguyên đều nằm trong ${LMS_OUTPUT_DIR}/<Config>.
    RUNTIME_OUTPUT_DIRECTORY  "${LMS_OUTPUT_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY  "${LMS_OUTPUT_DIR}/$<CONFIG>/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${LMS_OUTPUT_DIR}/$<CONFIG>/lib"
)

target_compile_definitions(LMS PRIVATE
    BUILD_MAJOR_VERSION=${LMS_VERSION_MAJOR}
    BUILD_MINOR_VERSION=${LMS_VERSION_MINOR}
    BUILD_PATCH_VERSION=${LMS_VERSION_PATCH}
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

if(MSVC)
    target_compile_options(LMS PRIVATE /utf-8 /W3 /MP)
endif()

target_link_libraries(LMS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Sql
    Qt6::Concurrent
)

# ---------------------------------------------------------------------------
# Tài nguyên runtime: copy cạnh exe sau mỗi lần build.
# Mã nguồn tìm "Default Files/LMS.db" và "Themes" theo applicationDirPath().
# ---------------------------------------------------------------------------
add_custom_command(TARGET LMS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LMS_DEFAULT_FILES_DIR}"
            "$<TARGET_FILE_DIR:LMS>/Default Files"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LMS_ASSETS_DIR}/Themes"
            "$<TARGET_FILE_DIR:LMS>/Themes"
    COMMENT "Copying runtime resources next to LMS.exe"
    VERBATIM
)

# ---------------------------------------------------------------------------
# Qt deploy sau build (Windows): kéo DLL Qt, plugin và CRT về cạnh exe.
# Ưu tiên target Qt6::windeployqt do Qt6CoreTools cung cấp; nếu không có thì
# tìm trong thư mục bin của Qt. Không tìm thấy thì cảnh báo, không làm hỏng build.
# ---------------------------------------------------------------------------
if(WIN32)
    if(TARGET Qt6::windeployqt)
        set(LMS_WINDEPLOYQT "$<TARGET_FILE:Qt6::windeployqt>")
    else()
        get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
        find_program(LMS_WINDEPLOYQT NAMES windeployqt windeployqt.exe HINTS "${_qt_bin}")
    endif()

    if(LMS_WINDEPLOYQT)
        add_custom_command(TARGET LMS POST_BUILD
            COMMAND "${LMS_WINDEPLOYQT}"
                    $<IF:$<CONFIG:Debug>,--debug,--release>
                    --compiler-runtime
                    --no-translations
                    "$<TARGET_FILE:LMS>"
            COMMENT "Running windeployqt"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt not found; Qt DLLs will not be copied next to LMS.exe")
    endif()
endif()

# ---------------------------------------------------------------------------
# Install: tạo thư mục phẳng để Inno Setup đóng gói (cmake --install).
# ---------------------------------------------------------------------------
install(TARGETS LMS
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION .
)
install(DIRECTORY "${LMS_DEFAULT_FILES_DIR}" DESTINATION .)
install(DIRECTORY "${LMS_ASSETS_DIR}/Themes" DESTINATION .)

qt_generate_deploy_app_script(
    TARGET LMS
    OUTPUT_SCRIPT LMS_DEPLOY_SCRIPT
    NO_UNSUPPORTED_PLATFORM_ERROR
    NO_TRANSLATIONS
)
install(SCRIPT ${LMS_DEPLOY_SCRIPT})
