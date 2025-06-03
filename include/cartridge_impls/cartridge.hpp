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
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC1Cartridge() = default;
};

class MBC1MCartridge final : public CoreCartridge {
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC1MCartridge() = default;
};
