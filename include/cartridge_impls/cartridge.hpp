#pragma once
#include "core/cartridge.hpp"
#include <cstdint>

class NoMBCCartridge final : public CoreCartridge {
public:
    NoMBCCartridge( std::vector<uint8_t>&& rom_ );
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;

    ~NoMBCCartridge() = default;
};

class MBC1Cartridge final : public CoreCartridge {
private:
    struct addresses {
        constexpr static uint16_t ramEnableStart         = 0x0000;
        constexpr static uint16_t ramEnableEnd           = 0x1FFF;
        constexpr static uint16_t romBankSelectStart     = 0x2000;
        constexpr static uint16_t romBankSelectEnd       = 0x3FFF;
        constexpr static uint16_t secondaryRegisterStart = 0x4000;
        constexpr static uint16_t secondaryRegisterEnd   = 0x5FFF;
        constexpr static uint16_t bankingModeSelectStart = 0x6000;
        constexpr static uint16_t bankingModeSelectEnd   = 0x7FFF;

        // MBC1M support up to 4 games each using 16 banks (256 KiB) of ROM
        // it means that nindento copyright header must be present in each of them
        // Check for the logo in the next game bank (0x10 = 16th bank)
        constexpr static uint16_t secondMBCMRomBank = 0x10;
    };

    bool isInRamEnableRange( const uint16_t address ) {
        return addresses::ramEnableStart <= address && address <= addresses::ramEnableEnd;
    }

    bool isInRomBankSelectRange( const uint16_t address ) {
        return addresses::romBankSelectStart <= address && address <= addresses::romBankSelectEnd;
    }

    bool isInSecondaryRegisterRange( const uint16_t address ) {
        return addresses::secondaryRegisterStart <= address && address <= addresses::secondaryRegisterEnd;
    }

    bool isInBankingModeSelectRange( const uint16_t address ) {
        return addresses::bankingModeSelectStart <= address && address <= addresses::bankingModeSelectEnd;
    }

    bool isMBCM          = false; // Flag to indicate if MBC1M is used
    bool ramEnabled      = false; // RAM enabled flag
    bool isInBankingMode = false; // Bank mode: 0 - simple, 1 - advanced
    bool alternateWiring = false; // Alternate wiring for MBC1M

    uint8_t selectedRomBankRegister = 0; // 5 bit register for selecting ROM bank
    uint8_t bankingRegister         = 0; // 2 bit register for selecting RAM bank or additional ROM bank

    constexpr static uint8_t selectedRomBankBitMask     = 0b11111;
    constexpr static uint8_t secondaryRegisterBitMask   = 0b11;
    constexpr static uint8_t selectedMBCMRomBankBitMask = 0b1111;

    constexpr static uint8_t ramEnableValue =
            0x0A; // if any other value is written to RAM enable register, RAM is disabled

    unsigned getRomBankIndex( bool isPrimaryRom ) const;
    uint8_t readRom( const uint16_t address, bool isPrimaryRom ) const;

public:
    MBC1Cartridge( std::vector<uint8_t>&& rom_ );
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC1Cartridge() = default;
};

class MBC2Cartridge final : public CoreCartridge {
private:
    // MBC2 has RAM consisting of 512 half-bytes
    constexpr static unsigned halfByteRamSize = 512;
    constexpr static uint8_t halfByteMask     = 0b1111;

    constexpr static uint8_t ramEnableValue =
            0x0A; // if any other value is written to RAM enable register, RAM is disabled

    uint8_t selectedRomBankRegister = 1;     // 4 bit register for selecting ROM bank
    bool ramEnabled                 = false; // RAM enabled flag

public:
    MBC2Cartridge( std::vector<uint8_t>&& rom_ );
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC2Cartridge() = default;
};
