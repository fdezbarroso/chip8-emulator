# Compiler
CC = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# SDL include and library paths
SDL_INCLUDE_PATH = SDL2/include
SDL_LIBRARY_PATH = SDL2/lib

# SDL flags
SDLFLAGS = -lmingw32 -lSDL2main -lSDL2

# Source files
SOURCES = src/emulator_utils.cpp src/instructions.cpp src/main.cpp src/sdl_utils.cpp

# Header files
HEADERS = src/chip8_constants.hpp src/chip8.hpp src/emulator_utils.hpp src/instructions.hpp src/limited_stack.hpp src/sdl_utils.hpp

#Executable name
EXECUTABLE = chip8

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CC) $(CXXFLAGS) -I$(SDL_INCLUDE_PATH) -L$(SDL_LIBRARY_PATH) -o $(EXECUTABLE) $(SOURCES) $(SDLFLAGS)

clean:
	del $(EXECUTABLE).exe
