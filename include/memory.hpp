#pragma once
#include <cstdint>

class Memory {
    enum rangeStart {
        //Numbers describe bank numbers
        ROM_00,
        ROM_01_TO_NN = 0x4000, // optional switchable bank via mapper
        VIDEO_RAM = 0x8000,    // in CGB mode switchable bank 0/1
        EXTERNAL_RAM = 0xA000, // optional switchable bank
        WORK_RAM_00 = 0xC000,
        WORK_RAM_01_TO_NN = 0xD000, // in CGB mode swichable bank 1-7
        ECHO_RAM = 0xE000,
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
        INTERRUPTS = 0xFF0F,
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
    uint8_t ROM00[16384];
    //16KB ROM bank01
    uint8_t videoRam[8192];
    //in CGB mode there is second vram bank
    //8KB External ram
    uint8_t workRam00[4096];
    uint8_t workRam0N[8192];

public:
    uint8_t& operator[]( const std::size_t index );
    const uint8_t& operator()( const std::size_t index ) const;
};
