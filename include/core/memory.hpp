#pragma once
#include "core/ICartridge.hpp"
#include <cstdint>

struct Memory {
    //To change cartridge, you have to turn off Gameboy
    uint64_t& tickNr;
    bool vramLock = 0;
    bool oamLock = 0;

    ICartridge& cartridge; // ROM + optional external RAM
    uint8_t videoRam[8192];
    uint8_t workRam00[4096];
    uint8_t workRam0N[4096];
    uint8_t oam[160];
    uint8_t ioRegisters[112]; // FF70-FF00, consider not allocating the gaps
    uint8_t interruptEnableRegister;
    uint8_t highRam[127];

    enum rangeStart {
        //Numbers describe bank numbers
        ROM_00,
        ROM_0N = 0x4000,       //optional switchable bank via mapper
        VIDEO_RAM = 0x8000,    //in CGB mode switchable bank 0/1
        EXTERNAL_RAM = 0xA000, //optional switchable bank
        WORK_RAM_00 = 0xC000,
        WORK_RAM_0N = 0xD000, //in CGB mode swichable bank 1-7
        ECHO_RAM_00 = 0xE000, //echo of WORK_RAM_00
        ECHO_RAM_0N = 0xF000, //echo of WORK_RAM_0N
        OBJECT_ATTRIBUTE_MEMORY = 0xFE00,
        NOT_USABLE = 0xFEA0,
        IO_REGISTERS = 0xFF00,
        HIGH_RAM = 0xFF80,
        INTERRUPT_ENABLE_REGISTER = 0xFFFF
    };
    enum ioRange {
        JOYPAD_INPUT = 0xFF00,
        SERIAL_TRANSFER_START = 0xFF01,
        SERIAL_TRANSFER_END = 0xFF02,
        DIVIDER_REGISTER = 0xFF04,
        TIMER_START = 0xFF05,
        TIMER_END = 0xFF07,
        INTERRUPT_FLAG = 0xFF0F,
        AUDIO_START = 0xFF10,
        AUDIO_END = 0xFF26,
        WAVE_PATTERN_START = 0xFF30,
        WAVE_PATTERN_END = 0xFF3F,
        LCD_START = 0xFF40,
        LCD_END = 0xFF4B,
        VRAM_BANK_SELECT = 0xFF4F, //CGB only
        BOOT_ROM_DISABLE_REGISTER = 0xFF50,
        //The rest is CGB only
        VRAM_DMA_START = 0xFF51,
        VRAM_DMA_END = 0xFF55,
        PALETES_START = 0xFF68,
        PALETES_END = 0xFF6B,
        WRAM_BANK_SELECT = 0xFF70
    };
    enum ppuRegisters {
        LCD_CONTROL = 0xFF40,
        LCD_STATUS = 0xFF41,
        BG_SCROLL_Y = 0xFF42,
        BG_SCROLL_X = 0xFF43,
        LCD_Y = 0xFF44,
        LYC = 0xFF45,
        // non-CGB mode only
        BG_PALETTE = 0xFF47,
        OBJECT_PALETTE_0 = 0xFF48,
        OBJECT_PALETTE_1 = 0xFF49,
        // both modes
        WIN_Y = 0xFF4A,
        WIN_X = 0xFF4B,
        // CGB mode only
        BG_PALETTE_SPEC = 0xFF68,
        BG_PALETTE_DATA = 0xFF69,
        OBJ_PALETTE_SPEC = 0xFF6A,
        OBJ_PALETTE_DATA = 0xFF6B
    };

    uint8_t read( const uint16_t index ) const;
    void write( const uint16_t index, uint8_t value );
    void setVramLock( bool locked );
    void setOamLock( bool locked );
    Memory( uint64_t& tickNr_, ICartridge& cartridge_ )
        : tickNr( tickNr_ )
        , cartridge( cartridge_ ) {
    }
};
