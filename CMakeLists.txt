cmake_minimum_required(VERSION 3.16)

project(chip8
    VERSION 2.0.0
    DESCRIPTION "CHIP-8 Emulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE_INIT "Release")

# Dependencies

# Qt6
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
)

if(Qt6_VERSION VERSION_GREATER_EQUAL "7.0")
    message(WARNING "Qt version ${Qt6_VERSION} is newer than tested. This project targets Qt 6.8 LTS.")
endif()

message(STATUS "Found Qt6 version: ${Qt6_VERSION}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/emulator_utils.cpp
    src/instructions.cpp
    src/main.cpp
    src/qt_utils.cpp
)

set(HEADERS
    src/qt_utils.hpp
)

# Executable
add_executable(chip8 ${SOURCES} ${HEADERS})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(chip8 PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(chip8 PRIVATE
        /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2 -DNDEBUG>
    )
endif()

target_link_libraries(chip8 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

# Platform-specific deployment
if(WIN32)
    # Windows: Use windeployqt to bundle Qt libraries
    get_target_property(QT6_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT6_WINDEPLOYQT_EXECUTABLE ${QT6_QMAKE_EXECUTABLE} PATH)
    set(QT6_WINDEPLOYQT_EXECUTABLE "${QT6_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    if(EXISTS ${QT6_WINDEPLOYQT_EXECUTABLE})
        add_custom_command(TARGET chip8 POST_BUILD
            COMMAND ${QT6_WINDEPLOYQT_EXECUTABLE} --no-translations $<TARGET_FILE:chip8>
            COMMENT "Deploying Qt libraries")
    else()
        message(WARNING "windeployqt.exe not found. You may need to manually deploy Qt libraries.")
    endif()

elseif(APPLE)
    # macOS: Use macdeployqt to create app bundle
    get_target_property(QT6_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT6_MACDEPLOYQT_EXECUTABLE ${QT6_QMAKE_EXECUTABLE} PATH)
    set(QT6_MACDEPLOYQT_EXECUTABLE "${QT6_MACDEPLOYQT_EXECUTABLE}/macdeployqt")

    if(EXISTS ${QT6_MACDEPLOYQT_EXECUTABLE})
        add_custom_command(TARGET chip8 POST_BUILD
            COMMAND ${QT6_MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE:chip8>
            COMMENT "Deploying Qt libraries for macOS")
    endif()

elseif(UNIX)
    # Linux: Set RPATH for Qt libraries (if installed in non-standard location)
    set_target_properties(chip8 PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()
