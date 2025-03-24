#pragma once
#include "core/cartridge.hpp"
#include <cstdint>
#include <ctime>
#include <span>
#include <vector>

class RaylibCartridge final : public CoreCartridge {
    class RTC {
        struct RTCRegisters {
            uint8_t seconds; // 0-59
            uint8_t minutes; // 0-59
            uint8_t hours;   // 0-23
            uint8_t dayLow;  // Lower 8 bits of day counter (0-255)

            uint8_t dayHigh; // Bit 0: Upper bit of day counter (0-1)
                             // Bit 6: Halt flag (0=active, 1=halted)
                             // Bit 7: Day counter carry bit
        };

        RTCRegisters rtcRegs {};
        RTCRegisters latchedRtcRegs {};
        bool rtcLatchedData = false;
        std::time_t rtcBaseTime = 0; // UNIX timestamp when RTC started/was last set
        bool rtcHalted = false;

    public:
        void updateRTC();
        uint8_t readRTC( uint8_t reg );
        void writeRTC( uint8_t reg, uint8_t value );
        void latchClockData( uint8_t value );
    };
    static constexpr auto romExtensions = { ".gb", ".sgb" };
    enum class CartridgeType : uint8_t {
        ROM_ONLY = 0x00,
        MBC1 = 0x01,
        MBC1_RAM = 0x02,
        MBC1_RAM_BATTERY = 0x03,
        MBC2 = 0x05,
        MBC2_BATTERY = 0x06,
        ROM_RAM = 0x08,
        ROM_RAM_BATTERY = 0x09,
        MMM01 = 0x0B,
        MMM01_RAM = 0x0C,
        MMM01_RAM_BATTERY = 0x0D,
        MBC3_TIMER_BATTERY = 0x0F,
        MBC3_TIMER_RAM_BATTERY = 0x10,
        MBC3 = 0x11,
        MBC3_RAM = 0x12,
        MBC3_RAM_BATTERY = 0x13,
        MBC5 = 0x19,
        MBC5_RAM = 0x1A,
        MBC5_RAM_BATTERY = 0x1B,
        MBC5_RUMBLE = 0x1C,
        MBC5_RUMBLE_RAM = 0x1D,
        MBC5_RUMBLE_RAM_BATTERY = 0x1E,
        MBC6 = 0x20,
        MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
        POCKET_CAMERA = 0xFC,
        BANDAI_TAMA5 = 0xFD,
        HUC3 = 0xFE,
        HUC1_RAM_BATTERY = 0xFF
    };
    std::vector<uint8_t> rom;
    std::span<uint8_t> romBank0N;
    std::span<uint8_t> ramBank;
    bool lockStart = false;
    uint8_t cbgFlag, cartridgeType, romSize, ramSize;
    bool alternateWiring = false;
    // registers
    bool enableRam = false;
    bool bankingMode = false;
    uint8_t romBankNr = 0, ramBankNr = 0;
    RTC rtc;

    void switchBankingMode( uint8_t value );
    void switchRomBank( uint8_t bank );
    void switchRamBank( uint8_t bank );

public:
    RaylibCartridge();
    ~RaylibCartridge() final = default;
    uint8_t read( const uint16_t address ) final;
    void write( const uint16_t address, uint8_t value ) final;
};
