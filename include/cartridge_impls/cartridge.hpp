#pragma once
#include "core/cartridge.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

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

class MBC3Cartridge final : public CoreCartridge {
private:
    class RTC {
    private:
        using SteadyClock = std::chrono::steady_clock;

        struct timeRegister {
            std::chrono::days days       = std::chrono::days( 0 );
            std::chrono::hours hours     = std::chrono::hours( 0 );
            std::chrono::minutes minutes = std::chrono::minutes( 0 );
            std::chrono::seconds seconds = std::chrono::seconds( 0 );
        };

        timeRegister latchedTime;
        timeRegister rtcTime;

        std::chrono::time_point<SteadyClock> referenceTime = SteadyClock::now();

        void captureRtcTime();
        void checkSetOverflow();

        struct bitMasks {
            static constexpr uint8_t dayHighDayCounter = 0b1;
            static constexpr uint8_t dayHighHalt       = 0b1000000;
            static constexpr uint8_t dayHighOverflow   = 0b10000000;
        };

    public:
        enum class Register : uint8_t {
            Seconds = 0x08,
            Minutes,
            Hours,
            DayLow,
            DayHigh,
        };

        bool isHalted      = true; // RTC is halted by default
        bool hasOverflowed = false;

        template<typename T>
        void updateTime( T newTime );
        void updateDay( uint8_t newDay, bool isDayHighRegister );
        void updateRegisters( uint8_t value );

        template<typename T>
        uint8_t getLatchTimeValue( bool isDayHighRegister = false ) const;

        void latchTime() {
            captureRtcTime();
            latchedTime = rtcTime;
        }
    };

    std::unique_ptr<RTC> rtc = nullptr;

    struct addresses {
        constexpr static uint16_t ramOrRtcEnableStart = 0x0000;
        constexpr static uint16_t ramOrRtcEnableEnd   = 0x1FFF;
        constexpr static uint16_t romBankSelectStart  = 0x2000;
        constexpr static uint16_t romBankSelectEnd    = 0x3FFF;
        constexpr static uint16_t ramOrRtcSelectStart = 0x4000;
        constexpr static uint16_t ramOrRtcSelectEnd   = 0x5FFF;
        constexpr static uint16_t rtcLatchStart       = 0x6000;
        constexpr static uint16_t rtcLatchEnd         = 0x7FFF;
    };

    bool isInRamOrRtcEnableRange( const uint16_t address ) const {
        return addresses::ramOrRtcEnableStart <= address && address <= addresses::ramOrRtcEnableEnd;
    }
    bool isInRomBankSelectRange( const uint16_t address ) const {
        return addresses::romBankSelectStart <= address && address <= addresses::romBankSelectEnd;
    }
    bool isInRamOrRTCSelectRange( const uint16_t address ) const {
        return addresses::ramOrRtcSelectStart <= address && address <= addresses::ramOrRtcSelectEnd;
    }
    bool isInRtcLatchRange( const uint16_t address ) const {
        return addresses::rtcLatchStart <= address && address <= addresses::rtcLatchEnd;
    }

    bool isValueInRtcRegisterRange( const uint8_t value ) const {
        return std::to_underlying( RTC::Register::Seconds ) <= value &&
               value <= std::to_underlying( RTC::Register::DayHigh );
    }
    bool isValueInRamBankRange( const uint8_t value ) const {
        return value < getRamBankCount();
    }
    bool isValueInRomBankRange( const uint8_t value ) const {
        return value < getRomBankCount();
    }

    constexpr static uint16_t rtcDayCounterMax = 0x1FF; // day register value after which overflow occurs
    constexpr static uint8_t ramEnableValue =
            0x0A; // if any other value is written to RAM enable register, RAM is disabled

    uint8_t readRom( const uint16_t address, bool isPrimaryRom ) const;

    uint8_t romSelectRegister          = 0;
    uint8_t lastLatchWriteValue        = 0; // maybe initialize to 0xFF?
    uint8_t ramBankOrRtcSelectRegister = 0;
    bool ramAndRtcEnabled              = false; // RAM enabled flag

public:
    MBC3Cartridge( std::vector<uint8_t>&& rom_, bool hasTimer_ = false );
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC3Cartridge() = default;
};
