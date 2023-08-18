#include <cstdint>
#include <array>
#include <stack>
#include <iostream>

template <typename T, std::size_t S>
class LimitedStack : public std::stack<T>
{
private:
    std::size_t max_size;

public:
    LimitedStack() : max_size(S) {}

    void push(const T &value)
    {
        if (this->size() >= max_size)
        {
            throw std::runtime_error("Stack max size exceded");
        }

        std::stack<T>::push(value);
    }
};

struct Chip8
{
    std::array<std::uint8_t, 16> registers{};
    std::array<std::uint8_t, 4096> memory{};
    std::uint16_t index_register{};

    std::uint16_t pc{};
    LimitedStack<std::uint16_t, 16> stack{};
    std::uint8_t sp{};

    std::uint8_t delay_timer{};
    std::uint8_t sound_timer{};

    std::array<std::uint8_t, 16> keys{};
    std::array<std::uint32_t, 64 * 32> display{};
};

int main()
{
    return 0;
}
