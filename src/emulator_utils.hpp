#ifndef EMULATOR_UTILS_HPP
#define EMULATOR_UTILS_HPP

#include <string>

#include "chip8.hpp"

// TODO: bool parse_arguments(int argc, char *argv[], ...);

// Loads the font into memory, starting at address 0x050 and finishing at 0x09F
void load_font(Chip8 &chip8);

// Loads the .ch8 ROM file's contents into memory when given a path to it
bool load_ROM(Chip8 &chip8, const std::string &rom_path);

// Decodes the opcode's intruction and calls the corresponding execution function
bool execute(Chip8 &chip8, const std::uint16_t &opcode);

#endif
