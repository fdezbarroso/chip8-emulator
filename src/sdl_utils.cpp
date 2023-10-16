#include "sdl_utils.hpp"

#include <iostream>

#include "chip8_constants.hpp"

bool initialize_SDL(SDL_Window **window, SDL_Renderer **renderer, const std::uint32_t &window_scale)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL could not initialize. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH * window_scale, WINDOW_HEIGHT * window_scale, SDL_WINDOW_SHOWN);

    if (*window == nullptr)
    {
        std::cerr << "Window could not be created. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE);

    if (*renderer == nullptr)
    {
        std::cerr << "Renderer could not be created. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Paint whole window black
    SDL_SetRenderDrawColor(*renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(*renderer);

    return true;
}

void clean_SDL(SDL_Window **window, SDL_Renderer **renderer)
{
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);
    SDL_Quit();
}

void render_display(Chip8 &chip8, SDL_Renderer **renderer, const std::uint32_t &window_scale)
{
    SDL_SetRenderDrawColor(*renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(*renderer);

    for (std::uint32_t x{0}; x < WINDOW_WIDTH; x++)
    {
        for (std::uint32_t y{0}; y < WINDOW_HEIGHT; y++)
        {
            std::uint8_t pixel_value = chip8.display.at(x + y * WINDOW_WIDTH);

            // Uses pixel_value for RGB as it should always be either 0x00 or 0xFF
            SDL_SetRenderDrawColor(*renderer, pixel_value, pixel_value, pixel_value, 0xFF);

            SDL_Rect pixel_scaled;
            pixel_scaled.x = x * window_scale;
            pixel_scaled.y = y * window_scale;
            pixel_scaled.w = window_scale;
            pixel_scaled.h = window_scale;
            SDL_RenderFillRect(*renderer, &pixel_scaled);
        }
    }

    SDL_RenderPresent(*renderer);
}

bool handle_input(Chip8 &chip8, const SDL_Event &e)
{
    switch (e.type)
    {
    case SDL_QUIT:
        return true;
        break;

    case SDL_KEYDOWN:
        switch (e.key.keysym.scancode)
        {
        // 1 -> 1
        case SDL_SCANCODE_1:
            chip8.keys.at(0x0) = 0x1;
            break;

        // 2 -> 2
        case SDL_SCANCODE_2:
            chip8.keys.at(0x1) = 0x1;
            break;

        // 3 -> 3
        case SDL_SCANCODE_3:
            chip8.keys.at(0x2) = 0x1;
            break;

        // 4 -> C
        case SDL_SCANCODE_4:
            chip8.keys.at(0x3) = 0x1;
            break;

        // Q -> 4
        case SDL_SCANCODE_Q:
            chip8.keys.at(0x4) = 0x1;
            break;

        // W -> 5
        case SDL_SCANCODE_W:
            chip8.keys.at(0x5) = 0x1;
            break;

        // E -> 6
        case SDL_SCANCODE_E:
            chip8.keys.at(0x6) = 0x1;
            break;

        // R -> D
        case SDL_SCANCODE_R:
            chip8.keys.at(0x7) = 0x1;
            break;

        // A -> 7
        case SDL_SCANCODE_A:
            chip8.keys.at(0x8) = 0x1;
            break;

        // S -> 8
        case SDL_SCANCODE_S:
            chip8.keys.at(0x9) = 0x1;
            break;

        // D -> 9
        case SDL_SCANCODE_D:
            chip8.keys.at(0xA) = 0x1;
            break;

        // F -> E
        case SDL_SCANCODE_F:
            chip8.keys.at(0xB) = 0x1;
            break;

        // Z -> A
        case SDL_SCANCODE_Z:
            chip8.keys.at(0xC) = 0x1;
            break;

        // X -> 0
        case SDL_SCANCODE_X:
            chip8.keys.at(0xD) = 0x1;
            break;

        // C -> B
        case SDL_SCANCODE_C:
            chip8.keys.at(0xE) = 0x1;
            break;

        // V -> F
        case SDL_SCANCODE_V:
            chip8.keys.at(0xF) = 0x1;
            break;

        default:
            break;
        }
        break;

    case SDL_KEYUP:
        switch (e.key.keysym.scancode)
        {
        // 1 -> 1
        case SDL_SCANCODE_1:
            chip8.keys.at(0x0) = 0x0;
            break;

        // 2 -> 2
        case SDL_SCANCODE_2:
            chip8.keys.at(0x1) = 0x0;
            break;

        // 3 -> 3
        case SDL_SCANCODE_3:
            chip8.keys.at(0x2) = 0x0;
            break;

        // 4 -> C
        case SDL_SCANCODE_4:
            chip8.keys.at(0x3) = 0x0;
            break;

        // Q -> 4
        case SDL_SCANCODE_Q:
            chip8.keys.at(0x4) = 0x0;
            break;

        // W -> 5
        case SDL_SCANCODE_W:
            chip8.keys.at(0x5) = 0x0;
            break;

        // E -> 6
        case SDL_SCANCODE_E:
            chip8.keys.at(0x6) = 0x0;
            break;

        // R -> D
        case SDL_SCANCODE_R:
            chip8.keys.at(0x7) = 0x0;
            break;

        // A -> 7
        case SDL_SCANCODE_A:
            chip8.keys.at(0x8) = 0x0;
            break;

        // S -> 8
        case SDL_SCANCODE_S:
            chip8.keys.at(0x9) = 0x0;
            break;

        // D -> 9
        case SDL_SCANCODE_D:
            chip8.keys.at(0xA) = 0x0;
            break;

        // F -> E
        case SDL_SCANCODE_F:
            chip8.keys.at(0xB) = 0x0;
            break;

        // Z -> A
        case SDL_SCANCODE_Z:
            chip8.keys.at(0xC) = 0x0;
            break;

        // X -> 0
        case SDL_SCANCODE_X:
            chip8.keys.at(0xD) = 0x0;
            break;

        // C -> B
        case SDL_SCANCODE_C:
            chip8.keys.at(0xE) = 0x0;
            break;

        // V -> F
        case SDL_SCANCODE_V:
            chip8.keys.at(0xF) = 0x0;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}
