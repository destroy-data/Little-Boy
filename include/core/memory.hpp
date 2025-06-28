#pragma once
#include "core/cartridge.hpp"
#include "core/core_constants.hpp"
#include <cstdint>

struct Memory {
    //To change cartridge, you have to turn off Gameboy
    bool vramLock = 0;
    bool oamLock  = 0;

    CoreCartridge* cartridge; // ROM + optional external RAM
    uint8_t videoRam[8192];
    uint8_t workRam00[4096];
    uint8_t workRam0N[4096];
    uint8_t oam[160];
    uint8_t ioRegisters[112]; // FF70-FF00, consider not allocating the gaps
    uint8_t interruptEnableRegister;
    uint8_t highRam[127];

    //helpers
    inline bool inRom( const uint16_t index ) const {
        return index < addr::videoRam;
    }
    bool inRom0N( const uint16_t index ) const {
        return addr::rom0N <= index and index < addr::videoRam;
    }
    bool inVideoRam( const uint16_t index ) const {
        return addr::videoRam <= index and index < addr::externalRam;
    }
    bool inExternalRam( const uint16_t index ) const {
        return addr::externalRam <= index and index < addr::workRam00;
    }
    bool inWorkRam00( const uint16_t index ) const {
        return addr::workRam00 <= index and index < addr::workRam0N;
    }
    bool inWorkRam0N( const uint16_t index ) const {
        return addr::workRam0N <= index and index < addr::echoRam00;
    }
    bool inEchoRam00( const uint16_t index ) const {
        return addr::echoRam00 <= index and index < addr::echoRam0N;
    }
    bool inEchoRam0N( const uint16_t index ) const {
        return addr::echoRam0N <= index and index < addr::objectAttributeMemory;
    }
    bool inObjectAttributeMemory( const uint16_t index ) const {
        return addr::objectAttributeMemory <= index and index < addr::notUsable;
    }
    bool inNotUsable( const uint16_t index ) const {
        return addr::notUsable <= index and index < addr::ioRegisters;
    }
    bool inIoRegisters( const uint16_t index ) const {
        return addr::ioRegisters <= index and index < addr::highRam;
    }
    bool inHighRam( const uint16_t index ) const {
        return addr::highRam <= index and index < addr::interruptEnableRegister;
    }

    uint8_t read( const uint16_t index ) const;
    void write( const uint16_t index, uint8_t value );
    void setVramLock( bool locked );
    void setOamLock( bool locked );
    Memory( CoreCartridge* cartridge_ );
};
