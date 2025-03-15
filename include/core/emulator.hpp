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

    uint64_t tickNr = 0;

public:
    Emulator() : mem( tickNr, cartridge ), cpu( tickNr, mem ), ppu( tickNr, mem ) {};
    int tick();
};
