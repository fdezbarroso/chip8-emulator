#include <stdlib.h>
#include <cstdint>
#include <array>
#include <stack>
#include <string>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>

template <typename T, std::size_t S>
class limited_stack : public std::stack<T>
{
private:
    std::size_t max_size;

public:
    limited_stack() : max_size(S) {}

    void push(const T &value)
    {
        if (this->size() >= max_size)
        {
            throw std::runtime_error("Stack max size exceded");
        }

        std::stack<T>::push(value);
    }
};

// Loads the font into memory, starting at address 0x050 and finishing at 0x09F
void load_font();
// Loads the .ch8 ROM file's contents into memory when given a path to it
bool load_ROM(const std::string rom_path);

const std::string ROM_LOCATION{"ROMs/logo.ch8"};
const unsigned int START_ADDRESS{0x200};

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE = 16;

const std::array<uint8_t, 80> font{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

std::array<std::uint8_t, 16> registers{};
std::array<std::uint8_t, 4096> memory{};
std::uint16_t index_register{};

std::uint16_t pc{};
limited_stack<std::uint16_t, 16> stack{};
std::uint8_t sp{};

std::uint8_t delay_timer{};
std::uint8_t sound_timer{};

std::array<std::uint8_t, 16> keys{};
std::array<std::uint32_t, 64 * 32> display{};

SDL_Window *window{nullptr};
SDL_Surface *screenSurface{nullptr};

int main(int argc, char *argv[])
{
    load_font();
    if (!load_ROM(ROM_LOCATION))
    {
        return EXIT_FAILURE;
    }

    pc = START_ADDRESS;
    uint16_t opcode{};

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (window == nullptr)
        {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return EXIT_FAILURE;
        }
        else
        {
            screenSurface = SDL_GetWindowSurface(window);

            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
            SDL_UpdateWindowSurface(window);

            SDL_Event e;
            bool quit = false;
            while (!quit)
            {
                while (SDL_PollEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                        quit = true;
                }

                opcode = memory.at(pc) << 8 | memory.at(pc + 1);
                pc += 2;

                SDL_UpdateWindowSurface(window);
            }
        }
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "Emulator terminated" << std::endl;

    return EXIT_SUCCESS;
}

void load_font()
{
    for (unsigned int i{0x050}; i <= 0x09F; i++)
        memory.at(i) = font.at(i - 0x050);
}

bool load_ROM(const std::string rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::binary);

    if (!rom_file)
    {
        std::cerr << "Failed to open the file at path: " << rom_path << std::endl;
        return false;
    }

    // Obtain ROM's contents size
    rom_file.seekg(0, rom_file.end);
    std::size_t rom_size{static_cast<std::size_t>(rom_file.tellg())};
    rom_file.seekg(0, rom_file.beg);

    if (rom_size > memory.size() - 0x200)
    {
        std::cerr << "ROM size exceeds available memory" << std::endl;
        return false;
    }

    // Reinterpret cast needed for std::uint8_t* -> char*
    if (!rom_file.read(reinterpret_cast<char *>(&memory.at(0x200)), rom_size))
    {
        std::cerr << "Failed to read from file at path: " << rom_path << std::endl;
        return false;
    }

    rom_file.close();
    return true;
}
