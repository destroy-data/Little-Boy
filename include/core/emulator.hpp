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
    static constexpr unsigned tickrate     = 4194304;
    static constexpr float oscillatoryTime = 1. / tickrate;

    unsigned tick() {
        const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
        auto cpuTicks             = cpu.tick();
        if( cpuDoubleSpeed )
            cpuTicks /= 2;
        for( auto i = 0u; i < cpuTicks; i++ ) {
            ppu.tick();
        }

        //apu.tick();
        return cpuTicks;
    }
    Emulator( std::unique_ptr<CoreCartridge>&& cartridge_ )
        : cartridge( std::move( cartridge_ ) )
        , memory( cartridge.get() )
        , cpu( memory )
        , ppu( memory ) {
    }
};
