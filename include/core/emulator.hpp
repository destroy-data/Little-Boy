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
    static constexpr unsigned tickrate     = 4'194'304;
    static constexpr float oscillatoryTime = 1. / tickrate;

    int tick() {
        cpu.tick();
        // const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
        for( int i = 0; i < 4; i++ ) {
            ppu.tick();
        }

        //apu.tick();
        return 4;
    }
    void handleJoypad() {
        cpu.handleJoypad();
    }
    Emulator( std::unique_ptr<CoreCartridge>&& cartridge_ )
        : cartridge( std::move( cartridge_ ) )
        , memory( cartridge.get() )
        , cpu( memory )
        , ppu( memory ) {
    }
};
