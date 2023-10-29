#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <SDL2/SDL.h>

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
    // Mute all sound
    bool mute{false};

    // CHIP-8 utils
    // Detect key release in opcode FX0A
    int key_pressed{-1};
    // Signal the need of rendering the screen
    bool render{false};
    // Store the precomputed sine values used for sound
    std::array<Sint16, BEEP_SAMPLE_RATE> sine_table{};
};

#endif
