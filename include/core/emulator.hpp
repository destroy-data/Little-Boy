#pragma once
#include "core/bus.hpp"
#include "core/core_constants.hpp"
#include "core/cpu.hpp"
#include "core/memory.hpp"
#include "core/timer.hpp"
#include <memory>

template<typename Tppu>
class Emulator final : public IBus {
    using JoypadHandler_t = void( IBus& );

    bool vramLocked = false;
    bool oamLocked  = false;

    bool inVideoRam( const uint16_t index ) const {
        return addr::videoRam <= index and index < addr::externalRam;
    }
    bool inObjectAttributeMemory( const uint16_t index ) const {
        return addr::objectAttributeMemory <= index and index < addr::notUsable;
    }
    bool inTimerRange( const uint16_t index ) {
        return addr::timer <= index and index <= addr::timerEnd;
    }

public:
    std::unique_ptr<CoreCartridge> cartridge;
    Timer timer { *this };
    Memory memory;
    Cpu cpu;
    Tppu ppu;
    JoypadHandler_t& joypadHandler;

    // IBus interface
    uint8_t read( uint16_t address ) const override {
        if( ( inVideoRam( address ) && vramLocked ) || ( inObjectAttributeMemory( address ) && oamLocked ) ) {
            [[unlikely]] return 0xFF;
        }
        return memory.read( address );
    }
    void write( uint16_t address, uint8_t value ) override {
        if( ( inVideoRam( address ) && vramLocked ) || ( inObjectAttributeMemory( address ) && oamLocked ) ) {
            [[unlikely]] return;
        }
        if( inTimerRange( address ) ) {
            [[unlikely]] timer.write( address, value );
            return;
        }
        memory.write( address, value );
    }

    void setOamLock( bool locked ) override {
        oamLocked = locked;
    }
    void setVramLock( bool locked ) override {
        vramLocked = locked;
    }

    SpriteAttribute getSpriteAttribute( uint8_t sprite_index ) const override {
        const uint16_t address = static_cast<uint16_t>( addr::objectAttributeMemory + sprite_index * 4 );
        return { .y         = memory.read( address ),
                 .x         = memory.read( address + 1 ),
                 .tileIndex = memory.read( address + 2 ),
                 .flags     = memory.read( address + 3 ) };
    }
    uint8_t directMemRead( uint16_t address ) const override {
        return memory.read( address );
    }
    virtual void directMemWrite( uint16_t address, uint8_t value ) override {
        memory.write( address, value );
    }


    unsigned tick() {
        unsigned ticks = cpu.tick();
        // const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
        for( unsigned i = 0; i < ticks; i++ ) {
            ppu.tick();
            timer.tick();
        }
        //apu.tick();
        return 4;
    }
    Emulator( std::unique_ptr<CoreCartridge>&& cartridge_, JoypadHandler_t& joypadHandler_ )
        : cartridge( std::move( cartridge_ ) )
        , memory( cartridge.get() )
        , cpu( *this )
        , ppu( *this )
        , joypadHandler( joypadHandler_ ) {
    }
};
