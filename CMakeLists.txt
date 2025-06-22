cmake_minimum_required(VERSION 3.14)

project(chip8
    VERSION 1.0.0
    DESCRIPTION "CHIP-8 Emulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE_INIT "Release")

# Dependencies
include(FetchContent)

set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
set(SDL_STATIC_ENABLED_BY_DEFAULT ON)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF)

set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_UNINSTALL ON)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.28.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL2)

# Source files
set(SOURCES
    src/emulator_utils.cpp
    src/instructions.cpp
    src/main.cpp
    src/sdl_utils.cpp
)

# Executable
add_executable(chip8 ${SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(chip8 PRIVATE
        -Wall 
        -Wextra
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2> 
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(chip8 PRIVATE 
        /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    )
endif()

target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main)

if(WIN32)
    set_target_properties(chip8 PROPERTIES WIN32_EXECUTABLE FALSE)
    
    if(MINGW)
        target_link_libraries(chip8 PRIVATE mingw32)
    endif()
endif()
