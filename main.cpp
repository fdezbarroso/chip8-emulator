#include <cstdint>
#include <array>
#include <stack>
#include <iostream>
#include <fstream>

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
void load_ROM(const char *rom_path);

const char *ROM_LOCATION{"ROMs\\logo.ch8"};
const unsigned int START_ADDRESS{0x200};

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

std::array<uint8_t, 80> font{
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

main()
{
    load_font();
    load_ROM(ROM_LOCATION);

    pc = START_ADDRESS;

    return 0;
}

void load_font()
{
    for (unsigned int i{0x050}; i <= 0x09F; i++)
        memory.at(i) = font.at(i - 0x050);
}

void load_ROM(const char *rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::binary);

    if (rom_file.is_open())
    {
        // Obtain ROM's contents size
        rom_file.seekg(0, rom_file.end);
        std::streampos rom_size{rom_file.tellg()};
        rom_file.seekg(0, rom_file.beg);

        // Cast needed for std::uint8_t* -> char*
        rom_file.read(reinterpret_cast<char *>(&memory.at(0x200)), rom_size);

        rom_file.close();
    }
    else
    {
        std::cerr << "Failed to open the file at path: " << rom_path << std::endl;
    }
}
