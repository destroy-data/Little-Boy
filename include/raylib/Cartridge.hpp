#pragma once
#include "core/ICartridge.hpp"
#include <cstdint>
#include <vector>

class Cartridge final : ICartridge {
    static constexpr auto romExtensions = { "gb", "sgb" };
    std::vector<uint8_t> romData;

public:
    Cartridge();
    uint8_t read( const uint16_t address ) final;
    void write( const uint16_t address, uint8_t value ) final;
};
