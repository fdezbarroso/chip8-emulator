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

    SDL_AudioDeviceID audio_device{};
    bool sound_playing{false};

    switch (parse_arguments(chip8, argc, argv, rom_location, cycle_frecuency, window_scale))
    {
    case -1:
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
        break;

    case 1:
        return EXIT_SUCCESS;
        break;

    default:
        break;
    }

    if (!initialize_SDL(chip8, &window, &renderer, window_scale, &audio_device))
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
    std::chrono::microseconds min_cycle_interval{1000000 / cycle_frecuency};
    std::chrono::microseconds min_timer_interval{1000000 / TIMER_FRECUENCY};

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
        current_time = std::chrono::high_resolution_clock::now();
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
                if (!chip8.mute && !sound_playing)
                {
                    SDL_PauseAudioDevice(audio_device, 0);
                    sound_playing = true;
                }
                chip8.sound_timer--;
            }
            else if (sound_playing)
            {
                SDL_PauseAudioDevice(audio_device, 1);
                sound_playing = false;
            }
        }
    }

    clean_SDL(&window, &renderer, &audio_device);

    return EXIT_SUCCESS;
}
