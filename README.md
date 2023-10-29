# chip8-emulator

A simple CHIP-8 emulator written in C++ and using the SDL2 library.

## Build

The included `Makefile` expects the SDL2 library to be found on the root of this repository, and for you to be on a Windows machine. Even though `C++ 20` is specified in the `Makefile`, `C++ 11` should be enough to compile it.

The emulator should work well on other operating systems since it doesn't use any Windows-specific features.

## Usage

Once built, a `chip8.exe` file will be generated. This expects the following arguments when being run:

 - `ROM file`: a string directing to a valid `.ch8` ROM file.
 - `Cycle delay`: an integer representing the speed at which the emulator should run, in instructions per second. For most ROMS, a cycle delay of 700 works best, but this is ROM-dependent.
 - `Window scale`: an integer representing the scale at which the pixels will be rendered. The original COSMAC VIP had a resolution of `64x32`, and since this is too small to be visible on modern monitors, this option will scale the image by the factor set here.

It also supports the following options, which should be added after the aforementioned arguments:

 - `--help`: shows basic support information about the usage of the emulator and stops its execution.
 - `--cosmac`: emulates some of the quirks of the original COSMAC VIP computer. It's recommended to turn it off for modern ROMs, but it's ROM dependent.
 - `--amiga`: emulates a quirk of the Amiga computer. It's recommended to turn it off except when running the original `Spacefight 2091!` ROM.
 - `--mute`: mutes the sound of the emulator.

An example command to run the emulator on the Windows 11 command line would be the following:
```
.\chip8.exe .\ROMs\Tetris.ch8 700 16 --mute
```

## Possible Improvements

This emulator is not perfect, and some know issues that could be improved on are the following:

 - Vertical Blank Interrupt (VBI) handling: since the SDL2 library is used, and with it a software renderer, there is no way to wait for vertical blanks when rendering to the screen, which can cause some screen tearing. This however should be barely noticeable.
 - Input reading: in multiple CHIP-8 emulators the input is read when a key is released, instead of when it is pressed. A simplified version of this is implemented when activating the `--cosmac` option, but it could be improved to actually read key releases of all valid keys.
 - Build system: the `Makefile` could be replaced by a `CMakeLists.txt` so SDL2 doesn't have to be added manually when building. This would also simplify the creation of builds for other operating systems.
 - User interface: adding a user interface would simplify the usage of the emulator greatly.

## License

This project is licensed under the MIT license, found in the root of this repository.