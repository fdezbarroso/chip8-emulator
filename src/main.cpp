#include <QApplication>
#include <iostream>
#include <string>

#include "emulator_utils.hpp"
#include "qt_utils.hpp"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    Chip8 chip8{};

    std::string rom_location{};
    std::uint32_t cycle_frecuency{};
    std::uint32_t window_scale{};

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

    load_font(chip8);

    if (!load_ROM(chip8, rom_location))
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        return EXIT_FAILURE;
    }

    Chip8EmulatorWidget emulator_widget(chip8, cycle_frecuency, window_scale);
    emulator_widget.setWindowTitle("CHIP-8 Emulator");
    emulator_widget.show();

    int execution_status = application.exec();
    if (execution_status != 0)
    {
        std::cerr << "Application exited with error code: " << execution_status << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
