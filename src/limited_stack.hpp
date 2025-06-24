#ifndef LIMITED_STACK_HPP
#define LIMITED_STACK_HPP

#include <stack>
#include <stdexcept>

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
            throw std::runtime_error("Stack max size exceded.");
        }

        std::stack<T>::push(value);
    }
};

#endif  // LIMITED_STACK_HPP
