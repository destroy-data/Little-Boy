#pragma once

template<typename Tcartridge, typename Tmemory, typename Tcpu, typename Tppu>
class Emulator {
public:
    Tcartridge cartridge;
    Tmemory memory;
    Tcpu cpu;
    Tppu ppu;

public:
    static constexpr unsigned tickrate = 4194304;
    static constexpr double oscillatoryTime = 1. / tickrate;
    Emulator() : memory( cartridge ), cpu( memory ), ppu( memory ) {
    }

    void tick();
};
