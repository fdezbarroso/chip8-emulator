# Compiler
CC = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++20

# SDL include and library paths
SDL_INCLUDE_PATH = SDL2/include
SDL_LIBRARY_PATH = SDL2/lib

# SDL flags
SDLFLAGS = -lmingw32 -lSDL2main -lSDL2

#Executable name
EXECUTABLE = chip8

all: $(EXECUTABLE)

$(EXECUTABLE): main.cpp
	$(CC) $(CXXFLAGS) -I$(SDL_INCLUDE_PATH) -L$(SDL_LIBRARY_PATH) -o $(EXECUTABLE) main.cpp $(SDLFLAGS)

clean:
	del $(EXECUTABLE).exe
