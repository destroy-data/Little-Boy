#pragma once
#include "core/cartridge.hpp"
#include <array>
#include <cstdint>

namespace addr {
// Numbers in names describe bank numbers
constexpr uint16_t rom00 = 0;
constexpr uint16_t rom0N = 0x4000;       //optional switchable bank via mapper
constexpr uint16_t videoRam = 0x8000;    //in CGB mode switchable bank 0/1
constexpr uint16_t externalRam = 0xA000; //optional switchable bank
constexpr uint16_t workRam00 = 0xC000;
constexpr uint16_t workRam0N = 0xD000; //in CGB mode swichable bank 1-7
constexpr uint16_t echoRam00 = 0xE000; //echo of WORK_RAM_00
constexpr uint16_t echoRam0N = 0xF000; //echo of WORK_RAM_0N
constexpr uint16_t objectAttributeMemory = 0xFE00;
constexpr uint16_t notUsable = 0xFEA0;
constexpr uint16_t ioRegisters = 0xFF00;
constexpr uint16_t highRam = 0xFF80;
constexpr uint16_t interruptEnableRegister = 0xFFFF;
// IO ranges
constexpr uint16_t joypadInput = 0xFF00;
constexpr uint16_t serialTransfer = 0xFF01;
constexpr uint16_t serialTransferEnd = 0xFF02;
constexpr uint16_t dividerRegister = 0xFF04;
constexpr uint16_t timer = 0xFF05;
constexpr uint16_t timerEnd = 0xFF07;
constexpr uint16_t interruptFlag = 0xFF0F;
constexpr uint16_t audio = 0xFF10;
constexpr uint16_t audioEnd = 0xFF26;
constexpr uint16_t wavePattern = 0xFF30;
constexpr uint16_t wavePatternEnd = 0xFF3F;
constexpr uint16_t lcd = 0xFF40;
constexpr uint16_t lcdEnd = 0xFF4B;
constexpr uint16_t vramBankSelect = 0xFF4F; //CGB only
constexpr uint16_t bootRomDisableRegister = 0xFF50;
// IO ranges - CGB only
constexpr uint16_t vramDma = 0xFF51;
constexpr uint16_t vramDmaEnd = 0xFF55;
constexpr uint16_t paletes = 0xFF68;
constexpr uint16_t paletesEnd = 0xFF6B;
constexpr uint16_t wramBankSelect = 0xFF70;
// Registers
constexpr uint16_t lcdControl = 0xFF40;
constexpr uint16_t lcdStatus = 0xFF41;
constexpr uint16_t bgScrollY = 0xFF42;
constexpr uint16_t bgScrollX = 0xFF43;
constexpr uint16_t lcdY = 0xFF44;
constexpr uint16_t lyc = 0xFF45;
constexpr uint16_t winY = 0xFF4A;
constexpr uint16_t winX = 0xFF4B;
// Registers - non-CGB mode only
constexpr uint16_t bgPalette = 0xFF47;
constexpr uint16_t objectPalette0 = 0xFF48;
constexpr uint16_t objectPalette1 = 0xFF49;
// Registers - CGB mode only
constexpr uint16_t bgPaletteSpec = 0xFF68;
constexpr uint16_t bgPaletteData = 0xFF69;
constexpr uint16_t objPaletteSpec = 0xFF6A;
constexpr uint16_t objPaletteData = 0xFF6B;
constexpr uint16_t key1 = 0xFF4D;
} // namespace addr

struct Memory {
    //To change cartridge, you have to turn off Gameboy
    bool vramLock = 0;
    bool oamLock = 0;

    CoreCartridge& cartridge; // ROM + optional external RAM
    uint8_t videoRam[8192];
    uint8_t workRam00[4096];
    uint8_t workRam0N[4096];
    uint8_t oam[160];
    uint8_t ioRegisters[112]; // FF70-FF00, consider not allocating the gaps
    uint8_t interruptEnableRegister;
    uint8_t highRam[127];

    //helpers
    bool inRom00( const uint16_t index ) const {
        return index < addr::rom0N;
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
    Memory( CoreCartridge& cartridge_ ) : cartridge( cartridge_ ) {
    }
};
