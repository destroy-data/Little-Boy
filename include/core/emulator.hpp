#pragma once
#include "core/memory.hpp"

template<typename Tcpu, typename Tppu>
class Emulator {
public:
    std::unique_ptr<CoreCartridge> cartridge;
    Memory memory;
    Tcpu cpu;
    Tppu ppu;

public:
    static constexpr unsigned tickrate      = 4194304;
    static constexpr double oscillatoryTime = 1. / tickrate;

    void tick();
    Emulator( std::unique_ptr<CoreCartridge>&& cartridge_ )
        : cartridge( std::move( cartridge_ ) )
        , memory( cartridge.get() )
        , cpu( memory )
        , ppu( memory ) {
    }
};
