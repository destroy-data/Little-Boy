#pragma once
#include "core/cartridge.hpp"

template<typename T>
class MBC1Cartridge final : public T {
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC1Cartridge() = default;
};

template<typename T>
class MBC1MCartridge final : public T {
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~MBC1MCartridge() = default;
};

// and so on (delete me, senpai)
