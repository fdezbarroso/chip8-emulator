#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <atomic>

#include "chip8_constants.hpp"
#include "limited_stack.hpp"

struct Chip8
{
    // CHIP-8 components
    std::array<std::uint8_t, 16> registers{};
    std::array<std::uint8_t, 4096> memory{};
    std::uint16_t index_register{};

    std::uint16_t pc{};
    // std::stack with a limited size of 16
    limited_stack<std::uint16_t, 16> stack{};

    std::uint8_t delay_timer{};
    std::uint8_t sound_timer{};

    std::array<std::uint8_t, 16> keys{};
    std::array<std::uint32_t, WINDOW_WIDTH * WINDOW_HEIGHT> display{};

    // CHIP-8 configuration options
    // Use original COSMAC VIP opcode interpretations
    bool cosmac{false};
    // Use Amiga opcode interpretations
    bool amiga{false};

    // CHIP-8 flags
    // Used to detect key release in opcode FX0A
    int wait_key_pressed{-1};
    // Used to signal the need of rendering the screen
    bool render{false};
    // Signals current beeping status
    std::atomic<bool> is_beeping{false};
};

#endif
