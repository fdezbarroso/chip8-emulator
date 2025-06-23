#include "instructions.hpp"

#include <algorithm>
#include <random>

#include "chip8_constants.hpp"

void op_00E0(Chip8 &chip8)
{
    std::fill(chip8.display.begin(), chip8.display.end(), 0);
    chip8.render = true;
}

void op_00EE(Chip8 &chip8)
{
    chip8.pc = chip8.stack.top();
    chip8.stack.pop();
}

void op_1NNN(Chip8 &chip8, const std::uint16_t opcode)
{
    chip8.pc = opcode & 0x0FFF;
}

void op_2NNN(Chip8 &chip8, const std::uint16_t opcode)
{
    chip8.stack.push(chip8.pc);
    chip8.pc = opcode & 0x0FFF;
}

void op_3XNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    if (chip8.registers.at(n2) == (opcode & 0x00FF))
    {
        chip8.pc += 2;
    }
}

void op_4XNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    if (chip8.registers.at(n2) != (opcode & 0x00FF))
    {
        chip8.pc += 2;
    }
}

void op_5XY0(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    if (chip8.registers.at(n2) == chip8.registers.at(n3))
    {
        chip8.pc += 2;
    }
}

void op_6XNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    chip8.registers.at(n2) = opcode & 0x00FF;
}

void op_7XNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    chip8.registers.at(n2) += opcode & 0x00FF;
}

void op_8XY0(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    chip8.registers.at(n2) = chip8.registers.at(n3);
}

void op_8XY1(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    chip8.registers.at(n2) |= chip8.registers.at(n3);

    if (chip8.cosmac)
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XY2(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    chip8.registers.at(n2) &= chip8.registers.at(n3);

    if (chip8.cosmac)
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XY3(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    chip8.registers.at(n2) ^= chip8.registers.at(n3);

    if (chip8.cosmac)
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XY4(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    std::uint16_t sum{static_cast<std::uint16_t>(chip8.registers.at(n2) + chip8.registers.at(n3))};

    chip8.registers.at(n2) = static_cast<std::uint8_t>(sum & 0xFF);

    if (sum > 0xFF)
    {
        chip8.registers.at(0xF) = 0x1;
    }
    else
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XY5(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    std::uint8_t v_x{chip8.registers.at(n2)}, v_y{chip8.registers.at(n3)};

    chip8.registers.at(n2) = v_x - v_y;

    if (v_x >= v_y)
    {
        chip8.registers.at(0xF) = 0x1;
    }
    else
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XY6(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    if (chip8.cosmac)
    {
        chip8.registers.at(n2) = chip8.registers.at(n3);
    }

    std::uint8_t v_x{chip8.registers.at(n2)};
    chip8.registers.at(n2) >>= 0x1;
    chip8.registers.at(0xF) = v_x & 0x1;
}

void op_8XY7(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    std::uint8_t v_x{chip8.registers.at(n2)}, v_y{chip8.registers.at(n3)};

    chip8.registers.at(n2) = v_y - v_x;

    if (v_y >= v_x)
    {
        chip8.registers.at(0xF) = 0x1;
    }
    else
    {
        chip8.registers.at(0xF) = 0x0;
    }
}

void op_8XYE(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    if (chip8.cosmac)
    {
        chip8.registers.at(n2) = chip8.registers.at(n3);
    }

    std::uint8_t v_x{chip8.registers.at(n2)};

    chip8.registers.at(n2) <<= 0x1;
    chip8.registers.at(0xF) = (v_x & 0x80) >> 0x7;
}

void op_9XY0(Chip8 &chip8, const std::uint8_t n2, const std::uint8_t n3)
{
    if (chip8.registers.at(n2) != chip8.registers.at(n3))
    {
        chip8.pc += 2;
    }
}

void op_ANNN(Chip8 &chip8, const std::uint16_t opcode)
{
    chip8.index_register = opcode & 0x0FFF;
}

void op_BNNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    // BNNN
    if (chip8.cosmac)
    {
        chip8.pc = (opcode & 0x0FFF) + chip8.registers.at(0x0);
    }
    // BXNN
    else
    {
        chip8.pc = (opcode & 0x0FFF) + chip8.registers.at(n2);
    }
}

void op_CXNN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0x0, 0xFF);

    chip8.registers.at(n2) = distr(gen) & (opcode & 0x00FF);
}

void op_DXYN(Chip8 &chip8, const std::uint16_t opcode, const std::uint8_t n2, const std::uint8_t n3)
{
    const std::uint8_t x_ini{static_cast<std::uint8_t>(chip8.registers.at(n2) % WINDOW_WIDTH)};
    const std::uint8_t y_ini{static_cast<std::uint8_t>(chip8.registers.at(n3) % WINDOW_HEIGHT)};
    const std::uint8_t height{static_cast<std::uint8_t>(opcode & 0x000F)};

    // VF set to 0 if no pixels are turned off
    chip8.registers.at(0xF) = 0x0;

    for (std::uint32_t y{0}; y < height; y++)
    {
        std::uint8_t sprite_data{chip8.memory.at(chip8.index_register + y)};

        // x from 0 to 7 since sprites are always 8 pixels wide
        for (std::uint32_t x{0}; x < 8; x++)
        {
            std::uint8_t sprite_bit{static_cast<std::uint8_t>((sprite_data >> (7 - x)) & 0x1)};
            std::uint32_t display_x{(x_ini + x)};
            std::uint32_t display_y{(y_ini + y)};

            if (!chip8.cosmac || (display_x < WINDOW_WIDTH && display_y < WINDOW_HEIGHT))
            {
                std::uint32_t display_index{(display_x % WINDOW_WIDTH) + (display_y % WINDOW_HEIGHT) * WINDOW_WIDTH};
                if (sprite_bit)
                {
                    if (chip8.registers.at(0xF) == 0x0 && chip8.display.at(display_index) == 0xFFFFFFFF)
                    {
                        // VF set to 1 if any pixels are turned off
                        chip8.registers.at(0xF) = 0x1;
                    }
                    chip8.display.at(display_index) ^= 0xFFFFFFFF;
                }
            }
        }
    }
    chip8.render = true;
}

void op_EX9E(Chip8 &chip8, const std::uint8_t n2)
{
    if (chip8.keys.at(chip8.registers.at(n2) & 0x0F) == 0x1)
    {
        chip8.pc += 2;
    }
}

void op_EXA1(Chip8 &chip8, const std::uint8_t n2)
{
    if (chip8.keys.at(chip8.registers.at(n2) & 0x0F) == 0x0)
    {
        chip8.pc += 2;
    }
}

void op_FX07(Chip8 &chip8, const std::uint8_t n2)
{
    chip8.registers.at(n2) = chip8.delay_timer;
}

void op_FX0A(Chip8 &chip8, const std::uint8_t n2)
{
    if (chip8.cosmac && chip8.key_pressed != -1)
    {
        if (chip8.keys.at(chip8.key_pressed) == 0x0)
        {
            chip8.registers.at(n2) = static_cast<uint8_t>(chip8.key_pressed);
            chip8.key_pressed = -1;
            return;
        }
    }

    for (std::uint8_t i{0}; chip8.key_pressed == -1 && i < chip8.keys.size(); i++)
    {
        if (chip8.keys.at(i) == 0x1)
        {
            if (chip8.cosmac)
            {
                chip8.key_pressed = i;
            }
            else
            {
                chip8.registers.at(n2) = i;
            }
            return;
        }
    }

    chip8.pc -= 2;
}

void op_FX15(Chip8 &chip8, const std::uint8_t n2)
{
    chip8.delay_timer = chip8.registers.at(n2);
}

void op_FX18(Chip8 &chip8, const std::uint8_t n2)
{
    chip8.sound_timer = chip8.registers.at(n2);
}

void op_FX1E(Chip8 &chip8, const std::uint8_t n2)
{
    std::uint16_t sum{static_cast<std::uint16_t>(chip8.index_register + chip8.registers.at(n2))};

    if (sum > 0x0FFF)
    {
        chip8.index_register = 0x0FFF;
        if (chip8.amiga)
        {
            chip8.registers.at(0xF) = 0x1;
        }
    }
    else
    {
        chip8.index_register = sum;
    }
}

void op_FX29(Chip8 &chip8, const std::uint8_t n2)
{
    chip8.index_register = FONT_ADDRESS + ((chip8.registers.at(n2) & 0x0F) * 0x5);
}

void op_FX33(Chip8 &chip8, const std::uint8_t n2)
{
    std::uint8_t val{chip8.registers.at(n2)};

    chip8.memory.at(chip8.index_register + 0x2) = val % 10;
    val /= 10;

    chip8.memory.at(chip8.index_register + 0x1) = val % 10;
    val /= 10;

    chip8.memory.at(chip8.index_register) = val % 10;
}

void op_FX55(Chip8 &chip8, const std::uint8_t n2)
{
    for (std::uint8_t i{0}; i <= n2; i++)
    {
        chip8.memory.at(chip8.index_register + i) = chip8.registers.at(i);
    }

    if (chip8.cosmac)
    {
        chip8.index_register = (chip8.index_register + n2 + 1) & 0x0FFF;
    }
}

void op_FX65(Chip8 &chip8, const std::uint8_t n2)
{
    for (std::uint8_t i{0}; i <= n2; i++)
    {
        chip8.registers.at(i) = chip8.memory.at(chip8.index_register + i);
    }

    if (chip8.cosmac)
    {
        chip8.index_register = (chip8.index_register + n2 + 1) & 0x0FFF;
    }
}
