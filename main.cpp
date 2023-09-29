#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <string>

#include <SDL2/SDL.h>

#include "limited_stack.hpp"

const std::string ROM_LOCATION{"ROMs/test_opcode.ch8"};
const std::uint32_t START_ADDRESS{0x200};
const std::uint32_t CYCLE_FRECUENCY{700};

const std::uint32_t SCALE{16};
const std::uint32_t WINDOW_WIDTH{64};
const std::uint32_t WINDOW_HEIGHT{32};

const bool COSMAC{false};

const std::array<uint8_t, 80> FONT{
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

// Initializes SDL and creates the emulator window
bool initialize_SDL();

// Loads the font into memory, starting at address 0x050 and finishing at 0x09F
void load_font();

// Loads the .ch8 ROM file's contents into memory when given a path to it
bool load_ROM(const std::string &rom_path);

// Decodes the opcode's intruction and calls the corresponding execution function
bool execute(const std::uint16_t &opcode);

// Loads the renderer with whatever is found on display
void render_display();

// INSTRUCTIONS
// TODO: modify to actual CHIP-8 names
void op_00E0();
void op_00EE();
void op_1NNN(const std::uint16_t &opcode);
void op_2NNN(const std::uint16_t &opcode);
void op_3XNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_4XNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_5XY0(const std::uint8_t &n2, const std::uint8_t &n3);
void op_6XNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_7XNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_8XY0(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY1(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY2(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY3(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY4(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY5(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY6(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XY7(const std::uint8_t &n2, const std::uint8_t &n3);
void op_8XYE(const std::uint8_t &n2, const std::uint8_t &n3);
void op_9XY0(const std::uint8_t &n2, const std::uint8_t &n3);
void op_ANNN(const std::uint16_t &opcode);
void op_DXYN(const std::uint16_t &opcode, const std::uint8_t &n2, const std::uint8_t &n3);

std::array<std::uint8_t, 16> registers{};
std::array<std::uint8_t, 4096> memory{};
std::uint16_t index_register{};

std::uint16_t pc{};
limited_stack<std::uint16_t, 16> stack{};
std::uint8_t sp{};

std::uint8_t delay_timer{};
std::uint8_t sound_timer{};

std::array<std::uint8_t, 16> keys{};
std::array<std::uint32_t, WINDOW_WIDTH * WINDOW_HEIGHT> display{};

SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};

int main(int argc, char *argv[])
{
    if (!initialize_SDL())
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
    }

    load_font();

    if (!load_ROM(ROM_LOCATION))
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
    }

    pc = START_ADDRESS;
    std::uint16_t opcode{};

    SDL_Event e;
    bool quit = false;

    std::chrono::high_resolution_clock::time_point last_cycle_time{std::chrono::high_resolution_clock::now()};
    std::chrono::microseconds min_cycle_interval{100000 / CYCLE_FRECUENCY};

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        std::chrono::high_resolution_clock::time_point current_cycle_time{std::chrono::high_resolution_clock::now()};
        std::chrono::microseconds time_since_last_cycle{std::chrono::duration_cast<std::chrono::microseconds>(current_cycle_time - last_cycle_time)};
        if (time_since_last_cycle >= min_cycle_interval)
        {
            last_cycle_time = current_cycle_time;

            // Form opcode from two contiguous memory locations
            opcode = memory.at(pc) << 8 | memory.at(pc + 1);
            pc += 2;

            if (!execute(opcode))
            {
                std::cerr << "Fatal error, execution aborted." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "Emulator terminated." << std::endl;

    return EXIT_SUCCESS;
}

bool initialize_SDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL could not initialize. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH * SCALE, WINDOW_HEIGHT * SCALE, SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        std::cerr << "Window could not be created. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Paint whole window black
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    return true;
}

void load_font()
{
    for (unsigned int i{0x050}; i <= 0x09F; i++)
    {
        memory.at(i) = FONT.at(i - 0x050);
    }
}

bool load_ROM(const std::string &rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::binary);

    if (!rom_file)
    {
        std::cerr << "Failed to open the file. Path: " << rom_path << std::endl;
        return false;
    }

    // Obtain ROM's contents size
    rom_file.seekg(0, rom_file.end);
    std::size_t rom_size{static_cast<std::size_t>(rom_file.tellg())};
    rom_file.seekg(0, rom_file.beg);

    if (rom_size > memory.size() - 0x200)
    {
        std::cerr << "ROM size exceeds available memory." << std::endl;
        return false;
    }

    // Reinterpret cast needed for std::uint8_t* -> char*
    if (!rom_file.read(reinterpret_cast<char *>(&memory.at(0x200)), rom_size))
    {
        std::cerr << "Failed to read from file. Path: " << rom_path << std::endl;
        return false;
    }

    rom_file.close();
    return true;
}

void render_display()
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    for (std::uint32_t x{0}; x < WINDOW_WIDTH; x++)
    {
        for (std::uint32_t y{0}; y < WINDOW_HEIGHT; y++)
        {
            std::uint8_t pixel_value = display.at(x + y * WINDOW_WIDTH);

            // Uses pixel_value for RGB as it should always be either 0x00 or 0xFF
            SDL_SetRenderDrawColor(renderer, pixel_value, pixel_value, pixel_value, 0xFF);

            SDL_Rect pixel_scaled;
            pixel_scaled.x = x * SCALE;
            pixel_scaled.y = y * SCALE;
            pixel_scaled.w = SCALE;
            pixel_scaled.h = SCALE;
            SDL_RenderFillRect(renderer, &pixel_scaled);
        }
    }

    SDL_RenderPresent(renderer);
}

bool execute(const std::uint16_t &opcode)
{
    // Extract hexadecimal nibbles
    std::uint8_t n1{}, n2{}, n3{}, n4{};
    n1 = opcode >> 12;
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
            op_00E0();
            break;

        case 0xE:
            std::cout << "RET" << std::endl;
            op_00EE();
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    // 1NNN
    case 0x1:
        std::cout << "JP addr" << std::endl;
        op_1NNN(opcode);
        break;

    // 2NNN
    case 0x2:
        std::cout << "CALL addr" << std::endl;
        op_2NNN(opcode);
        break;

    // 3XNN
    case 0x3:
        std::cout << "SE Vx, byte" << std::endl;
        op_3XNN(opcode, n2);
        break;

    // 4XNN
    case 0x4:
        std::cout << "SNE Vx, byte" << std::endl;
        op_4XNN(opcode, n2);
        break;

    // 5XY0
    case 0x5:
        if (n4 != 0x0)
        {
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }

        std::cout << "SE Vx, Vy" << std::endl;
        op_5XY0(n2, n3);
        break;

    // 6XNN
    case 0x6:
        std::cout << "LD Vx, byte" << std::endl;
        op_6XNN(opcode, n2);
        break;

    // 7XNN
    case 0x7:
        std::cout << "ADD Vx, byte" << std::endl;
        op_7XNN(opcode, n2);
        break;

    // 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE
    case 0x8:
        switch (n4)
        {
        case 0x0:
            std::cout << "LD Vx, Vy" << std::endl;
            op_8XY0(n2, n3);
            break;

        case 0x1:
            std::cout << "OR Vx, Vy" << std::endl;
            op_8XY1(n2, n3);
            break;

        case 0x2:
            std::cout << "AND Vx, Vy" << std::endl;
            op_8XY2(n2, n3);
            break;

        case 0x3:
            std::cout << "XOR Vx, Vy" << std::endl;
            op_8XY3(n2, n3);
            break;

        case 0x4:
            std::cout << "ADD Vx, Vy" << std::endl;
            op_8XY4(n2, n3);
            break;

        case 0x5:
            std::cout << "SUB Vx, Vy" << std::endl;
            op_8XY5(n2, n3);
            break;

        case 0x6:
            std::cout << "SHR Vx {, Vy}" << std::endl;
            op_8XY6(n2, n3);
            break;

        case 0x7:
            std::cout << "SUBN Vx, Vy" << std::endl;
            op_8XY7(n2, n3);
            break;

        case 0xE:
            std::cout << "SHL Vx {, Vy}" << std::endl;
            op_8XYE(n2, n3);
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
        op_9XY0(n2, n3);
        break;

    // ANNN
    case 0xA:
        std::cout << "LD I, addr" << std::endl;
        op_ANNN(opcode);
        break;

    // BNNN
    case 0xB:
        std::cout << "JP V0, addr" << std::endl;
        break;

    // CXNN
    case 0xC:
        std::cout << "RND Vx, byte" << std::endl;
        break;

    // DXYN
    case 0xD:
        std::cout << "DRW Vx, Vy, nibble" << std::endl;
        op_DXYN(opcode, n2, n3);
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

    // FX07, FX0A, FX15, FX18, FX1E, FX29, FX33, FX55, FX65
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

void op_00E0()
{
    std::fill(display.begin(), display.end(), 0);
    render_display();
}

void op_00EE()
{
    pc = stack.top();
    stack.pop();
}

void op_1NNN(const std::uint16_t &opcode)
{
    pc = opcode & 0x0FFF;
}

void op_2NNN(const std::uint16_t &opcode)
{
    stack.push(opcode);
    pc = opcode & 0x0FFF;
}

void op_3XNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    if (registers.at(n2) == (opcode & 0x00FF))
    {
        pc += 2;
    }
}

void op_4XNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    if (registers.at(n2) != (opcode & 0x00FF))
    {
        pc += 2;
    }
}

void op_5XY0(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (registers.at(n2) == registers.at(n3))
    {
        pc += 2;
    }
}

void op_6XNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    registers.at(n2) = opcode & 0x00FF;
}

void op_7XNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    registers.at(n2) += opcode & 0x00FF;
}

void op_8XY0(const std::uint8_t &n2, const std::uint8_t &n3)
{
    registers.at(n2) = registers.at(n3);
}

void op_8XY1(const std::uint8_t &n2, const std::uint8_t &n3)
{
    registers.at(n2) |= registers.at(n3);
}

void op_8XY2(const std::uint8_t &n2, const std::uint8_t &n3)
{
    registers.at(n2) &= registers.at(n3);
}

void op_8XY3(const std::uint8_t &n2, const std::uint8_t &n3)
{
    registers.at(n2) ^= registers.at(n3);
}

void op_8XY4(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (n2 + n3 > 0xFF)
    {
        registers.at(0xF) = 0x1;
    }
    else
    {
        registers.at(0xF) = 0x0;
    }

    registers.at(n2) += registers.at(n3);
}

void op_8XY5(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (n2 > n3)
    {
        registers.at(0xF) = 0x1;
    }
    else
    {
        registers.at(0xF) = 0x0;
    }

    registers.at(n2) = registers.at(n2) - registers.at(n3);
}

void op_8XY6(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (COSMAC)
    {
        registers.at(n2) = registers.at(n3);
    }

    registers.at(0xF) &= 0x000F;

    registers.at(n2) >>= 0x1;
}

void op_8XY7(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (n3 > n2)
    {
        registers.at(0xF) = 0x1;
    }
    else
    {
        registers.at(0xF) = 0x0;
    }

    registers.at(n2) = registers.at(n3) - registers.at(n2);
}

void op_8XYE(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (COSMAC)
    {
        registers.at(n2) = registers.at(n3);
    }

    registers.at(0xF) &= 0xF000;

    registers.at(n2) <<= 0x1;
}

void op_9XY0(const std::uint8_t &n2, const std::uint8_t &n3)
{
    if (registers.at(n2) != registers.at(n3))
    {
        pc += 2;
    }
}

void op_ANNN(const std::uint16_t &opcode)
{
    index_register = opcode & 0x0FFF;
}

void op_DXYN(const std::uint16_t &opcode, const std::uint8_t &n2, const std::uint8_t &n3)
{
    std::uint8_t x_ini{static_cast<std::uint8_t>(registers.at(n2) % WINDOW_WIDTH)};
    std::uint8_t y_ini{static_cast<std::uint8_t>(registers.at(n3) % WINDOW_HEIGHT)};
    std::uint8_t height{static_cast<std::uint8_t>(opcode & 0x000F)};

    // VF set to 0 if no pixels are turned off
    registers.at(0xF) = 0x00;

    for (std::uint32_t y{0}; y < height; y++)
    {
        std::uint8_t sprite_data{memory.at(index_register + y)};

        // x from 0 to 7 since sprites are always 8 pixels wide
        for (std::uint32_t x{0}; x < 8; x++)
        {
            std::uint8_t sprite_bit = (sprite_data >> (7 - x)) & 0x01;
            std::uint32_t display_index{x_ini + x + (y_ini + y) * WINDOW_WIDTH};

            if (sprite_bit)
            {
                if (!registers.at(0xF) && display.at(display_index))
                {
                    // VF set to 1 if any pixels are turned off
                    registers.at(0xF) = 0x01;
                }
                display.at(display_index) ^= 0xFFFFFFFF;
            }
        }
    }
    render_display();
}
