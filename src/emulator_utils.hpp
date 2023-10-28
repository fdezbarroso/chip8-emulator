#ifndef EMULATOR_UTILS_HPP
#define EMULATOR_UTILS_HPP

#include <string>

#include "chip8.hpp"

// Parses and handles the emulator arguments. Returns -1 on error, 0 on success, and 1 if the --help option is encountered
int parse_arguments(Chip8 &chip8, const int argc, const char *argv[], std::string &rom_location, std::uint32_t &cycle_frecuency, std::uint32_t &window_scale);

// Loads the font into memory, starting at address 0x050 and finishing at 0x09F
void load_font(Chip8 &chip8);

// Loads the .ch8 ROM file's contents into memory when given a path to it
bool load_ROM(Chip8 &chip8, const std::string &rom_path);

// Decodes the opcode's intruction and calls the corresponding execution function
bool execute(Chip8 &chip8, const std::uint16_t opcode);

#endif
