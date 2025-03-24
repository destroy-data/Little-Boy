#pragma once
#include <memory>
#include <tuple>

template<typename Tcartridge, typename Tmemory, typename Tcpu, typename Tppu>
class Emulator {
public:
    Tcartridge cartridge;
    Tmemory memory;
    Tcpu cpu;
    Tppu ppu;

public:
    Emulator() : memory( cartridge ), cpu( memory ), ppu( memory ) {
    }

    void tick() {
    }
};
