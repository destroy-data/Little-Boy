#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

class CoreCartridge {
public:
    constexpr static const char* cartridgeFilePatterns[2] = { "*.gb", "*.sgb" };

    enum class CartridgeType : uint8_t;
    enum class RomSize : uint8_t;
    enum class RamSize : uint8_t;

private:
    bool isValidRomSize( RomSize size );
    bool isValidRamSize( RamSize size );

    void initRom( const RomSize size );
    void initRam( const RamSize size );

protected:
    std::vector<uint8_t> rom;
    std::vector<std::span<uint8_t>> romBanks;
    std::vector<std::vector<uint8_t>> ramBanks;

    constexpr static uint16_t romBankSize     = 0x4000; // 16 KiB
    constexpr static uint16_t romStartAddress = 0x0000; // Start address for ROM bank

    constexpr static uint16_t ramBankSize     = 0x2000; // 8 KiB
    constexpr static uint16_t ramStartAddress = 0xA000; // Start address for RAM bank

    unsigned romBankCount = 0;
    unsigned ramBankCount = 0;

public:
    static std::unique_ptr<CoreCartridge> create( CartridgeType type, std::vector<uint8_t>&& rom );

    CoreCartridge() = default; //= delete;
    CoreCartridge( std::vector<uint8_t>&& rom_ );

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

    // clang-format off
    MBC6           = 0x20,
    MBC7SensorRuRB = 0x22, // MBC7 with Sensor, Rumble, RAM and Battery
    PocketCamera   = 0xFC,
    BandaiTama5    = 0xFD,
    HuC3           = 0xFE,
    HuC1RB         = 0xFF  // HuC1 with RAM and Battery
    // clang-format on
};

// Note:
// 32KiB * (RomSize + 1) = ROM size in bytes
// 2 ^ (RomSize + 1)     = number of 16KiB ROM banks
enum class CoreCartridge::RomSize : uint8_t {
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
    _1_5MiB         // 1.5 MiB
};

// Note:
// RAM consists of 8KiB banks
// 2 KiB chip was never used in a cartridges
enum class CoreCartridge::RamSize : uint8_t {
    // clang-format off
    _0KiB = 0x00,
    _2KiB,
    _8KiB,
    _32KiB,
    _128KiB,
    _64KiB
    // clang-format on
};

namespace addr {
constexpr uint16_t entryPointStart = 0x0100;
constexpr uint16_t entryPointEnd   = 0x0103;

constexpr uint16_t logoStart = 0x0104;
constexpr uint16_t logoEnd   = 0x0133;

constexpr uint16_t titleStart = 0x0134;
constexpr uint16_t titleEnd   = 0x0143;

constexpr uint16_t manufacturerCodeStart = 0x013F;
constexpr uint16_t manufacturerCodeEnd   = 0x0142;

constexpr uint16_t cgbFlag = 0x0143;

constexpr uint16_t newLicenseeCodeStart = 0x0144;
constexpr uint16_t newLicenseeCodeEnd   = 0x0145;

constexpr uint16_t sgbFlag         = 0x0146;
constexpr uint16_t cartridgeType   = 0x0147;
constexpr uint16_t romSize         = 0x0148;
constexpr uint16_t ramSize         = 0x0149;
constexpr uint16_t destinationCode = 0x014A;
constexpr uint16_t oldLicenseeCode = 0x014B;
constexpr uint16_t maskRomVersion  = 0x014C;

constexpr uint16_t headerChecksum      = 0x014D;
constexpr uint16_t globalChecksumStart = 0x014E;
constexpr uint16_t globalChecksumEnd   = 0x014F;
} // namespace addr
