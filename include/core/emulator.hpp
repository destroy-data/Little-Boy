#pragma once
#include "core/cpu.hpp"
#include "core/memory.hpp"
#include "raylib/Cartridge.hpp"
#include <cstdint>

class Emulator {
    Cartridge cartridge;
    Memory mem;
    CPU cpu;

    uint64_t tickNr = 0;

public:
    Emulator() : mem( tickNr, cartridge ), cpu( tickNr, mem ) {};
    int tick();
};
