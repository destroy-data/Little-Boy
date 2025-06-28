#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

class CoreCartridge {
public:
    constexpr static const char* cartridgeFilePatterns[2] = { "*.gb", "*.sgb" };

    enum class CartridgeType : uint8_t;

    enum class RomSizeByte : uint8_t;
    enum class RomSize : unsigned;

    enum class RamSizeByte : uint8_t;
    enum class RamSize : unsigned;

private:
    RomSize romSize { 0 };
    bool isValidRomSize( RomSizeByte size );
    void initRom( const RomSizeByte size );
    void setRomSize( const RomSizeByte size );

    RamSize ramSize { 0 };
    bool isValidRamSize( RamSizeByte size );
    void initRam( const RamSizeByte size );
    void setRamSize( const RamSizeByte size );

    std::vector<uint8_t> rom;

protected:
    std::vector<std::span<uint8_t>> romBanks;
    std::vector<std::vector<uint8_t>> ramBanks;

    constexpr static uint16_t romBankSize     = 0x4000;  // 16 KiB
    constexpr static uint16_t romStartAddress = 0x0000;  // Start address for ROM bank
    bool isInPrimaryRomRange( const uint16_t address ) { // 0x0000 <= address <= 0x3FFF
        return romStartAddress <= address && address < ( romStartAddress + romBankSize );
    }
    bool isInSecondaryRomRange( const uint16_t address ) { // 0x4000 <= address <= 0x7FFF
        return ( romStartAddress + romBankSize ) <= address && address < ( romStartAddress + 2 * romBankSize );
    }

    constexpr static uint16_t ramBankSize     = 0x2000; // 8 KiB
    constexpr static uint16_t ramStartAddress = 0xA000; // Start address for RAM bank
    bool isInRamRange( const uint16_t address ) {       // 0xA000 <= address <= 0xBFFF
        return ramStartAddress <= address && address < ( ramStartAddress + ramBankSize );
    }

    RomSize getRomSize() const {
        return romSize;
    }
    std::underlying_type_t<RomSize> getRomBankCount() const {
        return std::to_underlying( romSize );
    }

    RamSize getRamSize() const {
        return ramSize;
    }
    std::underlying_type_t<RamSize> getRamBankCount() const {
        return std::to_underlying( ramSize );
    }

    constexpr static uint8_t invalidReadValue = 0xFF; // Value returned on invalid read

    constexpr static uint8_t nintendoCopyrightHeader[] = {
            0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
            0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
            0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };

public:
    static std::unique_ptr<CoreCartridge> create( CartridgeType type, std::vector<uint8_t>&& rom );

    CoreCartridge() = default; //= delete;
    CoreCartridge( std::vector<uint8_t>&& rom_ );

    bool checkCopyRightHeader( uint16_t bankNumber ) const;

    virtual uint8_t read( const uint16_t address )                    = 0;
    virtual void write( const uint16_t address, const uint8_t value ) = 0;
    virtual ~CoreCartridge()                                          = default;
};

enum class CoreCartridge::CartridgeType : uint8_t {
    NoMBC = 0x00,

    MBC1,
    MBC1R,  // MBC1 with RAM
    MBC1RB, // MBC1 with RAM and Battery

    MBC2 = 0x05,
    MBC2B, // MBC2 with Battery

    RR = 0x08, // ROM + RAM
    RRB,       // ROM + RAM + Battery

    MMM01 = 0x0B,
    MMM01R,  // MMM01 with RAM
    MMM01RB, // MMM01 with RAM and Battery

    MBC3TB = 0x0F, // MBC3 with Timer and Battery
    MBC3TRB,       // MBC3 with Timer, RAM and Battery
    MBC3,          // MBC3
    MBC3R,         // MBC3 with RAM
    MBC3RB,        // MBC3 with RAM and Battery

    MBC5 = 0x19,
    MBC5R,    // MBC5 with RAM
    MBC5RB,   // MBC5 with RAM and Battery
    MBC5Ru,   // MBC5 with Rumble
    MBC5RuR,  // MBC5 with Rumble and RAM
    MBC5RuRB, // MBC5 with Rumble, RAM and Battery

    MBC6           = 0x20,
    MBC7SensorRuRB = 0x22, // MBC7 with Sensor, Rumble, RAM and Battery
    PocketCamera   = 0xFC,
    BandaiTama5    = 0xFD,
    HuC3           = 0xFE,
    HuC1RB         = 0xFF, // HuC1 with RAM and Battery
};

enum class CoreCartridge::RomSizeByte : uint8_t {
    _32KiB = 0x00,
    _64KiB,
    _128KiB,
    _256KiB,
    _512KiB,
    _1MiB,
    _2MiB,
    _4MiB,
    _8MiB,
    _1_1MiB = 0x52, // 1.1 MiB
    _1_2MiB,        // 1.2 MiB
    _1_5MiB,        // 1.5 MiB
};

// Note:
// Each number corresponds to the number of 16KiB ROM banks
enum class CoreCartridge::RomSize : unsigned {
    _32KiB  = 2,
    _64KiB  = 4,
    _128KiB = 8,
    _256KiB = 16,
    _512KiB = 32,
    _1MiB   = 64,
    _2MiB   = 128,
    _4MiB   = 256,
    _8MiB   = 512,
};

// Note:
// RAM consists of 8KiB banks
// 2 KiB chip was never used in a cartridges
enum class CoreCartridge::RamSizeByte : uint8_t {
    _0KiB = 0x00,
    _2KiB,
    _8KiB,
    _32KiB,
    _128KiB,
    _64KiB,
};

// Note:
// Each number corresponds to the number of 8KiB banks
enum class CoreCartridge::RamSize : unsigned {
    _0KiB   = 0,
    _8KiB   = 1,
    _32KiB  = 4,
    _64KiB  = 8,
    _128KiB = 16,
};
