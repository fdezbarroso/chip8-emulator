#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stack>
#include <stdlib.h>
#include <string>

#include <SDL2/SDL.h>

#include "limited_stack.hpp"

const std::string ROM_LOCATION{"ROMs/test_opcode.ch8"};
const std::uint32_t START_ADDRESS{0x200};
const std::uint32_t CYCLE_FRECUENCY{700};
const std::uint32_t TIMER_FRECUENCY{60};

const std::uint32_t SCALE{16};
const std::uint32_t WINDOW_WIDTH{64};
const std::uint32_t WINDOW_HEIGHT{32};

// Use original COSMAC VIP opcode interpretations
const bool COSMAC{false};

// Use Amiga opcode interpretations
const bool AMIGA{false};

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

// Loads the renderer with whatever is found on display
void render_display();

// Checks if an event corresponds to any of the valid inputs and processes it
// The valid inputs are:
// Quit -> When closing the window
// Keyboard press and release
// The Key equivalents on a QWERTY keyboard are:
// | 1 | 2 | 3 | 4 | -> | 1 | 2 | 3 | C |
// | Q | W | E | R | -> | 4 | 5 | 6 | D |
// | A | S | D | F | -> | 7 | 8 | 9 | E |
// | Z | X | C | V | -> | A | 0 | B | F |
bool handle_input(const SDL_Event &e);

// Decodes the opcode's intruction and calls the corresponding execution function
bool execute(const std::uint16_t &opcode);

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
void op_BNNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_CXNN(const std::uint16_t &opcode, const std::uint8_t &n2);
void op_DXYN(const std::uint16_t &opcode, const std::uint8_t &n2, const std::uint8_t &n3);
void op_EX9E(const std::uint8_t &n2);
void op_EXA1(const std::uint8_t &n2);
void op_FX07(const std::uint8_t &n2);
void op_FX0A(const std::uint8_t &n2);
void op_FX15(const std::uint8_t &n2);
void op_FX18(const std::uint8_t &n2);
void op_FX1E(const std::uint8_t &n2);
void op_FX29(const std::uint8_t &n2);
void op_FX33(const std::uint8_t &n2);
void op_FX55(const std::uint8_t &n2);
void op_FX65(const std::uint8_t &n2);

std::array<std::uint8_t, 16> registers{};
std::array<std::uint8_t, 4096> memory{};
std::uint16_t index_register{};

std::uint16_t pc{};
limited_stack<std::uint16_t, 16> stack{};
// TODO: probably remove sp
std::uint8_t sp{};

std::uint8_t delay_timer{};
std::uint8_t sound_timer{};

std::array<std::uint8_t, 16> keys{};
std::array<std::uint32_t, WINDOW_WIDTH * WINDOW_HEIGHT> display{};

SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};

// Used to detect key release in opcode FX0A
int wait_key_pressed{-1};

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
    std::chrono::high_resolution_clock::time_point last_timer_time{std::chrono::high_resolution_clock::now()};
    std::chrono::microseconds min_cycle_interval{100000 / CYCLE_FRECUENCY};
    std::chrono::microseconds min_timer_interval{100000 / TIMER_FRECUENCY};

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            quit = handle_input(e);
        }

        std::chrono::high_resolution_clock::time_point current_time{std::chrono::high_resolution_clock::now()};
        std::chrono::microseconds time_since_last_cycle{std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_cycle_time)};
        if (time_since_last_cycle >= min_cycle_interval)
        {
            last_cycle_time = current_time;

            // Form opcode from two contiguous memory locations
            opcode = memory.at(pc) << 8 | memory.at(pc + 1);
            pc += 2;

            if (!execute(opcode))
            {
                std::cerr << "Fatal error, execution aborted." << std::endl;
                return EXIT_FAILURE;
            }
        }

        // Update timers if needed
        std::chrono::microseconds time_since_last_timer{std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_timer_time)};
        if (time_since_last_timer >= min_timer_interval)
        {
            last_timer_time = current_time;

            if (delay_timer)
            {
                delay_timer--;
            }
            if (sound_timer)
            {
                // TODO: Play beep
                sound_timer--;
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

bool handle_input(const SDL_Event &e)
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
            keys.at(0x0) = 0x1;
            break;

        // 2 -> 2
        case SDL_SCANCODE_2:
            keys.at(0x1) = 0x1;
            break;

        // 3 -> 3
        case SDL_SCANCODE_3:
            keys.at(0x2) = 0x1;
            break;

        // 4 -> C
        case SDL_SCANCODE_4:
            keys.at(0x3) = 0x1;
            break;

        // Q -> 4
        case SDL_SCANCODE_Q:
            keys.at(0x4) = 0x1;
            break;

        // W -> 5
        case SDL_SCANCODE_W:
            keys.at(0x5) = 0x1;
            break;

        // E -> 6
        case SDL_SCANCODE_E:
            keys.at(0x6) = 0x1;
            break;

        // R -> D
        case SDL_SCANCODE_R:
            keys.at(0x7) = 0x1;
            break;

        // A -> 7
        case SDL_SCANCODE_A:
            keys.at(0x8) = 0x1;
            break;

        // S -> 8
        case SDL_SCANCODE_S:
            keys.at(0x9) = 0x1;
            break;

        // D -> 9
        case SDL_SCANCODE_D:
            keys.at(0xA) = 0x1;
            break;

        // F -> E
        case SDL_SCANCODE_F:
            keys.at(0xB) = 0x1;
            break;

        // Z -> A
        case SDL_SCANCODE_Z:
            keys.at(0xC) = 0x1;
            break;

        // X -> 0
        case SDL_SCANCODE_X:
            keys.at(0xD) = 0x1;
            break;

        // C -> B
        case SDL_SCANCODE_C:
            keys.at(0xE) = 0x1;
            break;

        // V -> F
        case SDL_SCANCODE_V:
            keys.at(0xF) = 0x1;
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
            keys.at(0x0) = 0x0;
            break;

        // 2 -> 2
        case SDL_SCANCODE_2:
            keys.at(0x1) = 0x0;
            break;

        // 3 -> 3
        case SDL_SCANCODE_3:
            keys.at(0x2) = 0x0;
            break;

        // 4 -> C
        case SDL_SCANCODE_4:
            keys.at(0x3) = 0x0;
            break;

        // Q -> 4
        case SDL_SCANCODE_Q:
            keys.at(0x4) = 0x0;
            break;

        // W -> 5
        case SDL_SCANCODE_W:
            keys.at(0x5) = 0x0;
            break;

        // E -> 6
        case SDL_SCANCODE_E:
            keys.at(0x6) = 0x0;
            break;

        // R -> D
        case SDL_SCANCODE_R:
            keys.at(0x7) = 0x0;
            break;

        // A -> 7
        case SDL_SCANCODE_A:
            keys.at(0x8) = 0x0;
            break;

        // S -> 8
        case SDL_SCANCODE_S:
            keys.at(0x9) = 0x0;
            break;

        // D -> 9
        case SDL_SCANCODE_D:
            keys.at(0xA) = 0x0;
            break;

        // F -> E
        case SDL_SCANCODE_F:
            keys.at(0xB) = 0x0;
            break;

        // Z -> A
        case SDL_SCANCODE_Z:
            keys.at(0xC) = 0x0;
            break;

        // X -> 0
        case SDL_SCANCODE_X:
            keys.at(0xD) = 0x0;
            break;

        // C -> B
        case SDL_SCANCODE_C:
            keys.at(0xE) = 0x0;
            break;

        // V -> F
        case SDL_SCANCODE_V:
            keys.at(0xF) = 0x0;
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
        op_BNNN(opcode, n2);
        break;

    // CXNN
    case 0xC:
        std::cout << "RND Vx, byte" << std::endl;
        op_CXNN(opcode, n2);
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
            op_EX9E(n2);
            break;

        case 0xA:
            if (n4 != 0x1)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "SKNP Vx" << std::endl;
            op_EXA1(n2);
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
                op_FX07(n2);
                break;

            case 0xA:
                std::cout << "LD Vx, K" << std::endl;
                op_FX0A(n2);
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
                op_FX15(n2);
                break;

            case 0x8:
                std::cout << "LD ST, Vx" << std::endl;
                op_FX18(n2);
                break;

            case 0xE:
                std::cout << "ADD I, Vx" << std::endl;
                op_FX1E(n2);
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
            op_FX29(n2);
            break;

        case 0x3:
            if (n4 != 0x3)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD B, Vx" << std::endl;
            op_FX33(n2);
            break;

        case 0x5:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD [I], Vx" << std::endl;
            op_FX55(n2);
            break;

        case 0x6:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                op_FX65(n2);
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
    stack.push(pc);
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

void op_BNNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    // BNNN
    if (COSMAC)
    {
        pc = (opcode & 0x0FFF) + registers.at(0x0);
    }
    // BXNN
    else
    {
        pc = (opcode & 0x0FFF) + registers.at(n2);
    }
}

void op_CXNN(const std::uint16_t &opcode, const std::uint8_t &n2)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0x0, 0xFF);

    registers.at(n2) = distr(gen) & (opcode & 0x00FF);
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

void op_EX9E(const std::uint8_t &n2)
{
    if (keys.at(registers.at(n2)) == 0x1)
    {
        pc += 2;
    }
}

void op_EXA1(const std::uint8_t &n2)
{
    if (keys.at(registers.at(n2)) == 0x0)
    {
        pc += 2;
    }
}

void op_FX07(const std::uint8_t &n2)
{
    registers.at(n2) = delay_timer;
}

void op_FX0A(const std::uint8_t &n2)
{
    if (COSMAC && wait_key_pressed != -1)
    {
        if (!keys.at(wait_key_pressed))
        {
            registers.at(wait_key_pressed) = wait_key_pressed;
            wait_key_pressed = -1;
            return;
        }
    }

    for (std::uint8_t i{0}; wait_key_pressed == -1 && i < keys.size(); i++)
    {
        if (keys.at(i))
        {
            if (COSMAC)
            {
                wait_key_pressed = i;
            }
            else
            {
                registers.at(n2) = i;
            }
            return;
        }
    }

    pc -= 2;
}

void op_FX15(const std::uint8_t &n2)
{
    delay_timer = registers.at(n2);
}

void op_FX18(const std::uint8_t &n2)
{
    sound_timer = registers.at(n2);
}

void op_FX1E(const std::uint8_t &n2)
{
    index_register += registers.at(n2);

    if (AMIGA)
    {
        if (index_register > 0x0FFF)
        {
            registers.at(0xF) = 0x1;
        }
    }
}

void op_FX29(const std::uint8_t &n2)
{
    // TODO: Investigate further on COSMAC specific implementation
    index_register = 0x050 + (registers.at(n2) * 0x5);
}

void op_FX33(const std::uint8_t &n2)
{
    std::uint8_t val{registers.at(n2)};

    memory.at(index_register + 0x2) = val % 10;
    val /= 10;

    memory.at(index_register + 0x1) = val % 10;
    val /= 10;

    memory.at(index_register) = val % 10;
}

void op_FX55(const std::uint8_t &n2)
{
    for (std::uint8_t i{0}; i <= n2; i++)
    {
        memory.at(index_register + i) = registers.at(i);
    }
}

void op_FX65(const std::uint8_t &n2)
{
    for (std::uint8_t i{0}; i <= n2; i++)
    {
        registers.at(i) = memory.at(index_register + i);
    }
}
