#include "emulator_utils.hpp"

#include <iostream>
#include <fstream>

#include "instructions.hpp"

int parse_arguments(Chip8 &chip8, int argc, char *argv[], std::string &rom_location, std::uint32_t &cycle_frecuency, std::uint32_t &window_scale)
{
    std::string emulator_usage{"Usage: /path/to/chip8.exe /path/to/rom<string> cycle_delay<int> window_scale<int> --cosmac(optional) --amiga(optional)"};

    for (int i{1}; i < argc; i++)
    {
        std::string arg{argv[i]};
        if (arg == "--help" || arg == "-h")
        {
            std::cout << "A simple CHIP-8 emulator.\n"
                      << emulator_usage << std::endl;
            return 1;
        }
    }

    if (argc < 4)
    {
        std::cerr << "Not enough arguments.\n"
                  << emulator_usage << std::endl;
        return -1;
    }

    rom_location = argv[1];

    try
    {
        cycle_frecuency = std::stoi(argv[2]);
    }
    catch (std::invalid_argument &)
    {
        std::cerr << "Invalid cycle_delay argument.\n"
                  << emulator_usage << std::endl;
        return -1;
    }

    try
    {
        window_scale = std::stoi(argv[3]);
    }
    catch (std::invalid_argument &)
    {
        std::cerr << "Invalid window_scale argument.\n"
                  << emulator_usage << std::endl;
        return -1;
    }

    for (int i{4}; i < argc; i++)
    {
        std::string arg{argv[i]};
        if (arg == "--cosmac")
        {
            chip8.cosmac = true;
        }
        if (arg == "--amiga")
        {
            chip8.amiga = true;
        }
    }
    return 0;
}

void load_font(Chip8 &chip8)
{
    for (std::uint32_t i{FONT_ADDRESS}; i <= 0x09F; i++)
    {
        chip8.memory.at(i) = FONT.at(i - FONT_ADDRESS);
    }
}

bool load_ROM(Chip8 &chip8, const std::string &rom_path)
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

    if (rom_size > chip8.memory.size() - 0x200)
    {
        std::cerr << "ROM size exceeds available memory." << std::endl;
        return false;
    }

    // Reinterpret cast needed for std::uint8_t* -> char*
    if (!rom_file.read(reinterpret_cast<char *>(&(chip8.memory.at(0x200))), rom_size))
    {
        std::cerr << "Failed to read from file. Path: " << rom_path << std::endl;
        return false;
    }

    rom_file.close();
    return true;
}

bool execute(Chip8 &chip8, const std::uint16_t &opcode)
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
            op_00E0(chip8);
            break;

        case 0xE:
            std::cout << "RET" << std::endl;
            op_00EE(chip8);
            break;

        default:
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }
        break;

    // 1NNN
    case 0x1:
        std::cout << "JP addr" << std::endl;
        op_1NNN(chip8, opcode);
        break;

    // 2NNN
    case 0x2:
        std::cout << "CALL addr" << std::endl;
        op_2NNN(chip8, opcode);
        break;

    // 3XNN
    case 0x3:
        std::cout << "SE Vx, byte" << std::endl;
        op_3XNN(chip8, opcode, n2);
        break;

    // 4XNN
    case 0x4:
        std::cout << "SNE Vx, byte" << std::endl;
        op_4XNN(chip8, opcode, n2);
        break;

    // 5XY0
    case 0x5:
        if (n4 != 0x0)
        {
            std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
            return false;
        }

        std::cout << "SE Vx, Vy" << std::endl;
        op_5XY0(chip8, n2, n3);
        break;

    // 6XNN
    case 0x6:
        std::cout << "LD Vx, byte" << std::endl;
        op_6XNN(chip8, opcode, n2);
        break;

    // 7XNN
    case 0x7:
        std::cout << "ADD Vx, byte" << std::endl;
        op_7XNN(chip8, opcode, n2);
        break;

    // 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE
    case 0x8:
        switch (n4)
        {
        case 0x0:
            std::cout << "LD Vx, Vy" << std::endl;
            op_8XY0(chip8, n2, n3);
            break;

        case 0x1:
            std::cout << "OR Vx, Vy" << std::endl;
            op_8XY1(chip8, n2, n3);
            break;

        case 0x2:
            std::cout << "AND Vx, Vy" << std::endl;
            op_8XY2(chip8, n2, n3);
            break;

        case 0x3:
            std::cout << "XOR Vx, Vy" << std::endl;
            op_8XY3(chip8, n2, n3);
            break;

        case 0x4:
            std::cout << "ADD Vx, Vy" << std::endl;
            op_8XY4(chip8, n2, n3);
            break;

        case 0x5:
            std::cout << "SUB Vx, Vy" << std::endl;
            op_8XY5(chip8, n2, n3);
            break;

        case 0x6:
            std::cout << "SHR Vx {, Vy}" << std::endl;
            op_8XY6(chip8, n2, n3);
            break;

        case 0x7:
            std::cout << "SUBN Vx, Vy" << std::endl;
            op_8XY7(chip8, n2, n3);
            break;

        case 0xE:
            std::cout << "SHL Vx {, Vy}" << std::endl;
            op_8XYE(chip8, n2, n3);
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
        op_9XY0(chip8, n2, n3);
        break;

    // ANNN
    case 0xA:
        std::cout << "LD I, addr" << std::endl;
        op_ANNN(chip8, opcode);
        break;

    // BNNN / BXNN
    case 0xB:
        std::cout << "JP V0, addr" << std::endl;
        op_BNNN(chip8, opcode, n2);
        break;

    // CXNN
    case 0xC:
        std::cout << "RND Vx, byte" << std::endl;
        op_CXNN(chip8, opcode, n2);
        break;

    // DXYN
    case 0xD:
        std::cout << "DRW Vx, Vy, nibble" << std::endl;
        op_DXYN(chip8, opcode, n2, n3);
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
            op_EX9E(chip8, n2);
            break;

        case 0xA:
            if (n4 != 0x1)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "SKNP Vx" << std::endl;
            op_EXA1(chip8, n2);
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
                op_FX07(chip8, n2);
                break;

            case 0xA:
                std::cout << "LD Vx, K" << std::endl;
                op_FX0A(chip8, n2);
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
                op_FX15(chip8, n2);
                break;

            case 0x8:
                std::cout << "LD ST, Vx" << std::endl;
                op_FX18(chip8, n2);
                break;

            case 0xE:
                std::cout << "ADD I, Vx" << std::endl;
                op_FX1E(chip8, n2);
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
            op_FX29(chip8, n2);
            break;

        case 0x3:
            if (n4 != 0x3)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD B, Vx" << std::endl;
            op_FX33(chip8, n2);
            break;

        case 0x5:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD [I], Vx" << std::endl;
            op_FX55(chip8, n2);
            break;

        case 0x6:
            if (n4 != 0x5)
            {
                std::cerr << "Invalid instruction. Opcode: " << std::hex << opcode << std::endl;
                return false;
            }

            std::cout << "LD Vx, [I]" << std::endl;
            op_FX65(chip8, n2);
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
