# chip8-emulator

A simple CHIP-8 emulator written in C++ and using the SDL2 library.

## Build

This project uses `CMake` for cross-platform building and automatically downloads `SDL2` version `2.28.3` via FetchContent, so no manual library setup is required.

### Prerequisites
- **CMake 3.14 or higher.**
- **C++ compiler** supporting C++11 (GCC, Clang, or MSVC).
- **Git** for downloading SDL2.
- **Internet connection** for the first build only, to download dependencies (SDL2).

### Building

1. Clone the repository through `HTTPS`:
    ```bash
    git clone https://github.com/fdezbarroso/chip8-emulator.git
    cd chip8-emulator
    ```
    Or using `SSH`
    ```bash
    git clone git@github.com:fdezbarroso/chip8-emulator.git
    cd chip8-emulator
    ```
2. Create a build directory and configure:
    ```bash
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```
    On Windows using MinGW:
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
    ```
3. Build the project:
    ```bash
    cmake --build .
    ```
4. The executable will be located at `build/bin/chip8.exe` (Windows) or `build/bin/chip8` (Unix).

**Note**: The first build will take longer as it downloads and compiles necessary dependencies. Subsequent builds will be much faster.

### Build Types
- **Release** (default): Optimized for performance.
- **Debug**: Includes debug symbols, slower execution.
  ```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  ```

The emulator should work on Windows, Linux and macOS without platform-specific modifications.

## Usage

Once built, or downloaded, you'll be left with a `chip8.exe`/`chip8` file inside the `build/bin/` directory. This expects the following <ins>*ordered*</ins> command-line arguments when run:

 - **ROM file**: a string path pointing to a valid `.ch8` ROM file.
 - **Cycle delay**: an integer representing the speed at which the emulator should run, in instructions per second. For most ROMS, a cycle delay of 700 works best, but this is ROM-dependent.
 - **Window scale**: an integer representing the scale at which the pixels will be rendered. The original COSMAC VIP had a resolution of `64x32`, and since this is too small to be usable on modern monitors, the emulator will scale the image by the factor set here.

It also supports the following options, which should be added after the aforementioned arguments:

 - **--help**: shows basic support information about the usage of the emulator and stops its execution.
 - **--cosmac**: emulates some of the quirks of the original COSMAC VIP computer. It's recommended to turn it off for modern ROMs, but it depends on a case by case basis.
 - **--amiga**: emulates a quirk of the Amiga computer. It's recommended to keep it turned off, except when running the original `Spacefight 2091!` ROM.
 - **--mute**: mutes the sound of the emulator.

An example command to run the emulator on the Windows 11 command line would be the following:
```
# From the build directory
./bin/chip8.exe ../ROMs/Tetris.ch8 700 16

# Or from the project root
./build/bin/chip8.exe ./ROMs/Tetris.ch8 700 16
```
**Disclaimer**: different CHIP-8 ROMs have different requirements. It's recommended to try out multiple command-line argument setups to achieve optimal results.

## Possible Improvements

This emulator is not perfect, and some know issues and features that could be worked on are the following:

 - **Vertical Blank Interrupt (VBI) handling**: since the SDL2 library is used, and with it a software renderer, there is no way to wait for vertical blanks when rendering to the screen, which can cause some screen tearing. This however should be barely noticeable.
 - **Input reading**: in multiple CHIP-8 emulators the input is read when a key is released, instead of when it is pressed. A simplified version of this is implemented when activating the `--cosmac` option, but it could be improved to actually read key releases of all valid keys. This would be specially useful on ROMs like Hidden by David Winter, or any other that uses multiple `Fx0A` consecutive opcodes to handle input.
 - ~~**Build system**: the `Makefile` could be replaced by a `CMakeLists.txt` so SDL2 doesn't have to be added manually when building. This would also simplify the creation of builds for other operating systems.~~ Already done! :D
 - **User interface**: adding a user interface would simplify the usage of the emulator greatly.
 - **SUPER-CHIP** and **XO-CHIP** support.

## Examples

#### Here are some examples of the emulator in action!

| ![Tetris](/src/imgs/Tetris.gif) |
| :-----------------------------: |
|    *Tetris [Fran Dachille]*     |

| ![Pong](/src/imgs/Pong.gif) |
| :-------------------------: |
|   *Pong [Paul Vervalin]*    |

| ![Trip8](/src/imgs/Trip8.gif)  |
| :----------------------------: |
| *Trip8 Demo [Revival Studios]* |

#### And here are some test screenshots. Following the *very* useful [Timendus test suite](https://github.com/Timendus/chip8-test-suite).

| ![Timendus Corax test](/src/imgs/corax_test.png) |
| :----------------------------------------------: |
|         *Opcodes test [Corax, Timendus]*         |

| ![Timendus flags test](/src/imgs/flags_test.png) |
| :----------------------------------------------: |
|              *Flag test [Timendus]*              |

| ![Timendus quirks test](/src/imgs/quirks_test.png) |
| :------------------------------------------------: |
|              *Quirks test [Timendus]*              |
*This test's fail is explained in the [Possible Improvements](#possible-improvements) section.*

## Resources

 - A repository with many CHIP-8 ROMs by Revival Studios: [chip8-roms](https://github.com/kripod/chip8-roms/tree/master)
 - Timendus test suite: [chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
 - Cowgod's technical reference: [Cowgod's documentation](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
 - Laurence Scotford's very useful guide on sprite rendering: [Laurence's guide](https://www.laurencescotford.net/2020/07/19/chip-8-on-the-cosmac-vip-drawing-sprites/)
 - Tobias V. Langhoff's CHIP-8 guide, very in-depth: [Tobias' guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
 - Austin Morlan's CHIP-8 guide, great to get started: [Austin's guide](https://austinmorlan.com/posts/chip8_emulator/)
 - Alastair Robertson's web emulator, very useful web emulator to compare against: [Alastair's emulator](https://ajor.co.uk/chip8/)

## License

This project is licensed under the MIT license, found in the root of this repository.