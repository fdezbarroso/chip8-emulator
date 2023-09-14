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
bool load_ROM(const std::string &rom_path);
// Decodes the opcode's intruction and calls the corresponding execution function
bool execute(const std::uint16_t &opcode);

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
        std::cerr << "Fatal error, execution aborted" << std::endl;
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
        window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);

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

                if (!execute(opcode))
                {
                    std::cerr << "Fatal error, execution aborted" << std::endl;
                    return EXIT_FAILURE;
                }

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

bool load_ROM(const std::string &rom_path)
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

bool execute(const std::uint16_t &opcode)
{
    // Extract hexadecimal nibbles
    std::uint16_t n1{}, n2{}, n3{}, n4{};
    n1 = (opcode >> 12) & 0xF;
    n2 = (opcode >> 8) & 0xF;
    n3 = (opcode >> 4) & 0xF;
    n4 = opcode & 0xF;

    // Decode instruction and call corresponding function
    switch (n1)
    {
    // 00E0, 00EE
    // 0NNN not implemented
    case 0x0:
        if (n2 != 0x0 || n3 != 0xE)
        {
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }

        switch (n4)
        {
        case 0x0:
            std::cout << "CLS" << std::endl;
            break;

        case 0xE:
            std::cout << "RET" << std::endl;
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    // 1NNN
    case 0x1:
        std::cout << "JP addr" << std::endl;
        break;

    // 2NNN
    case 0x2:
        std::cout << "CALL addr" << std::endl;
        break;

    // 3XKK
    case 0x3:
        std::cout << "SE Vx, byte" << std::endl;
        break;

    // 4XKK
    case 0x4:
        std::cout << "SNE Vx, byte" << std::endl;
        break;

    // 5XY0
    case 0x5:
        if (n4 != 0x0)
        {
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }

        std::cout << "SE Vx, Vy" << std::endl;
        break;

    // 6XKK
    case 0x6:
        std::cout << "LD Vx, byte" << std::endl;
        break;

    // 7XKK
    case 0x7:
        std::cout << "ADD Vx, byte" << std::endl;
        break;

    // 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE
    case 0x8:
        switch (n4)
        {
        case 0x0:
            std::cout << "LD Vx, Vy" << std::endl;
            break;

        case 0x1:
            std::cout << "OR Vx, Vy" << std::endl;
            break;

        case 0x2:
            std::cout << "AND Vx, Vy" << std::endl;
            break;

        case 0x3:
            std::cout << "XOR Vx, Vy" << std::endl;
            break;

        case 0x4:
            std::cout << "ADD Vx, Vy" << std::endl;
            break;

        case 0x5:
            std::cout << "SUB Vx, Vy" << std::endl;
            break;

        case 0x6:
            std::cout << "SHR Vx {, Vy}" << std::endl;
            break;

        case 0x7:
            std::cout << "SUBN Vx, Vy" << std::endl;
            break;

        case 0xE:
            std::cout << "SHL Vx {, Vy}" << std::endl;
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    // 9XY0
    case 0x9:
        if (n4 != 0x0)
        {
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }

        std::cout << "SNE Vx, Vy" << std::endl;
        break;

    // ANNN
    case 0xA:
        std::cout << "LD I, addr" << std::endl;
        break;

    // BNNN
    case 0xB:
        std::cout << "JP V0, addr" << std::endl;
        break;

    // CXKK
    case 0xC:
        std::cout << "RND Vx, byte" << std::endl;
        break;

    // DXYN
    case 0xD:
        std::cout << "DRW Vx, Vy, nibble" << std::endl;
        break;

    // EX9E, EXA1
    case 0xE:
        switch (n3)
        {
        case 0x9:
            if (n4 != 0xE)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "SKP Vx" << std::endl;
            break;

        case 0xA:
            if (n4 != 0x1)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "SKNP Vx" << std::endl;
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    // Fx07, Fx0A, Fx15, Fx18, Fx1E, Fx29, Fx33, Fx55, Fx65
    case 0xF:
        switch (n3)
        {
        case 0x0:
            switch (n4)
            {
            case 0x7:
                std::cout << "LD Vx, DT" << std::endl;
                break;

            case 0xA:
                std::cout << "LD Vx, K" << std::endl;
                break;

            default:
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }
            break;

        case 0x1:
            switch (n4)
            {
            case 0x5:
                std::cout << "LD DT, Vx" << std::endl;
                break;

            case 0x8:
                std::cout << "LD ST, Vx" << std::endl;
                break;

            case 0xE:
                std::cout << "ADD I, Vx" << std::endl;
                break;

            default:
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }
            break;

        case 0x2:
            if (n4 != 0x9)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD F, Vx" << std::endl;
            break;

        case 0x3:
            if (n4 != 0x3)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD B, Vx" << std::endl;
            break;

        case 0x5:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD [I], Vx" << std::endl;
            break;

        case 0x6:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD Vx, [I]" << std::endl;
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    default:
        std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
        return false;
    }
    return true;
}
