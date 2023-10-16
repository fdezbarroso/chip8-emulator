#include <chrono>
#include <iostream>
#include <string>

#include "emulator_utils.hpp"
#include "sdl_utils.hpp"

int main(int argc, char *argv[])
{
    Chip8 chip8{};
    std::string rom_location{};
    std::uint32_t cycle_frecuency{};

    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};
    std::uint32_t window_scale{};

    // TODO: modify with parse_arguments function call
    std::string emulator_usage{"Usage: /path/to/chip8.exe /path/to/rom<string> cycle_delay<int> window_scale<int> --cosmac(optional) --amiga(optional)"};

    for (int i{1}; i < argc; i++)
    {
        std::string arg{argv[i]};
        if (arg == "--help" || arg == "-h")
        {
            std::cout << "A simple CHIP-8 emulator\n"
                      << emulator_usage << std::endl;
            return EXIT_SUCCESS;
        }
    }

    if (argc < 4)
    {
        std::cout << "Not enough arguments\n"
                  << emulator_usage << std::endl;
        return EXIT_FAILURE;
    }

    rom_location = argv[1];

    try
    {
        cycle_frecuency = std::stoi(argv[2]);
    }
    catch (std::invalid_argument &)
    {
        std::cout << "Invalid cycle_delay argument\n"
                  << emulator_usage << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        window_scale = std::stoi(argv[3]);
    }
    catch (std::invalid_argument &)
    {
        std::cout << "Invalid window_scale argument\n"
                  << emulator_usage << std::endl;
        return EXIT_FAILURE;
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

    if (!initialize_SDL(&window, &renderer, window_scale))
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
    }

    load_font(chip8);

    if (!load_ROM(chip8, rom_location))
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
    }

    chip8.pc = START_ADDRESS;
    std::uint16_t opcode{};

    SDL_Event e;
    bool quit = false;

    std::chrono::high_resolution_clock::time_point last_cycle_time{std::chrono::high_resolution_clock::now()};
    std::chrono::high_resolution_clock::time_point last_timer_time{std::chrono::high_resolution_clock::now()};
    std::chrono::microseconds min_cycle_interval{100000 / cycle_frecuency};
    std::chrono::microseconds min_timer_interval{100000 / TIMER_FRECUENCY};

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            quit = handle_input(chip8, e);
        }

        std::chrono::high_resolution_clock::time_point current_time{std::chrono::high_resolution_clock::now()};
        std::chrono::microseconds time_since_last_cycle{std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_cycle_time)};
        if (time_since_last_cycle >= min_cycle_interval)
        {
            last_cycle_time = current_time;

            // Form opcode from two contiguous memory locations
            opcode = chip8.memory.at(chip8.pc) << 8 | chip8.memory.at(chip8.pc + 1);
            chip8.pc += 2;

            if (!execute(chip8, opcode))
            {
                std::cerr << "Fatal error, execution aborted." << std::endl;
                return EXIT_FAILURE;
            }

            if (chip8.render)
            {
                render_display(chip8, &renderer, window_scale);
                chip8.render = false;
            }
        }

        // Update timers if needed
        std::chrono::microseconds time_since_last_timer{std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_timer_time)};
        if (time_since_last_timer >= min_timer_interval)
        {
            last_timer_time = current_time;

            if (chip8.delay_timer)
            {
                chip8.delay_timer--;
            }
            if (chip8.sound_timer)
            {
                // TODO: Play beep
                chip8.sound_timer--;
            }
        }
    }

    clean_SDL(&window, &renderer);

    std::cout << "Emulator terminated." << std::endl;

    return EXIT_SUCCESS;
}
