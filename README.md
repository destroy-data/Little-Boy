### Little boy – Gameboy emulator

**If you're reading this on Github - it's only a mirror. Primary repository is here: <https://gitlab.com/destroy-data/gameboy-emulator>**

This is gameboy emulator hobby project. It's going to consist of core, which is platform-agnostic, and platform-specific "front-ends". That is, all emulation is done in core, platform-specific code only cares about I/O like display and speaker.
I want emulator to work at least on desktop, rpi pico, STM32f401 ( aka 64KB RAM version of Blackpill ) and in WebAssembly. Also in case of microcontrollers, it should be possible to play with cartridges, not only ROM images.

Due to microcontrollers hardware limitations, code is written with low RAM usage in mind. By that I mean mainly avoiding large buffers like for example frame buffer in core and µc specific code; not squeezing out every possible byte.
In future I plan to extend functionality to also include Gameboy Color. This is **work in progress**.
