#ifndef SDL_UTILS_HPP
#define SDL_UTILS_HPP

#include <SDL2/SDL.h>

#include "chip8.hpp"

// Initializes SDL and creates the emulator window
bool initialize_SDL(SDL_Window **window, SDL_Renderer **renderer, const std::uint32_t &window_scale);

// Cleans SDL components
void clean_SDL(SDL_Window **window, SDL_Renderer **renderer);

// Loads the renderer with whatever is found on the chip8's display
void render_display(Chip8 &chip8, SDL_Renderer **renderer, const std::uint32_t &window_scale);

// Checks if an event corresponds to any of the valid inputs and processes it
// The valid inputs are:
// Quit -> When closing the window
// Keyboard press and release
// The Key equivalents on a QWERTY keyboard are:
// | 1 | 2 | 3 | 4 | -> | 1 | 2 | 3 | C |
// | Q | W | E | R | -> | 4 | 5 | 6 | D |
// | A | S | D | F | -> | 7 | 8 | 9 | E |
// | Z | X | C | V | -> | A | 0 | B | F |
bool handle_input(Chip8 &chip8, const SDL_Event &e);

#endif
