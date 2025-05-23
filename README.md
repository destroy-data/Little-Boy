# Little boy – Gameboy emulator

**If you're reading this on Github - it's only a mirror. Primary repository is here: <https://gitlab.com/destroy-data/gameboy-emulator>**

This is a gameboy emulator hobby project. It's going to consist of a core, which is platform-agnostic, and platform-specific "front-ends". That is, all emulation is done in the core, platform-specific code only cares about I/O like display and speaker.
I want the emulator to work at least on desktop, rpi pico, STM32f401 ( aka 64KB RAM version of Blackpill ) and in WebAssembly. Also in case of microcontrollers, it should be possible to play with cartridges, not only ROM images.

Due to microcontroller hardware limitations, code is written with low RAM usage in mind. By that I mean mainly avoiding large buffers like for example frame buffer in the core and µc specific code; not squeezing out every possible byte.
In the future I plan to extend functionality to also include Gameboy Color. This is **work in progress, not usable for now**.

### How to build
    mkdir build
    cd build
    cmake ..
    make -j $nproc

voilà  
You can add `-DBUILD_TESTS=OFF` to cmake command to skip building tests. Binaries are located in build/bin.

### Dependencies
All dependencies are fetched by cmake, those are:
- [raylib](https://www.raylib.com/)
- [Catch2](https://github.com/catchorg/Catch2)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)

Thanks to all the authors <3  
You can find all dependencies' licenses in the external_licenses directory.

### Little Boy's license
This software is licensed under the BSD Zero Clause License; its full text is in the LICENSE file in the project root.
