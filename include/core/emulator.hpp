#pragma once
#include "core/cpu.hpp"
#include "core/memory.hpp"
#include "core/ppu.hpp"
#include "raylib/Cartridge.hpp"
#include <cstdint>

class Emulator {
public:
    Cartridge cartridge;
    Memory mem;
    CPU cpu;
    PPU ppu;

public:
    Emulator() : mem( cartridge ), cpu( mem ), ppu( mem ) {};
    int tick();
};
