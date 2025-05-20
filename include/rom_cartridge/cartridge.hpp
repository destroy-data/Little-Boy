#pragma once
#include "core/cartridge.hpp"

class ROMCartridge final : public CoreCartridge {
    uint8_t read( const uint16_t address ) override;
    void write( const uint16_t address, const uint8_t value ) override;
    ~ROMCartridge() = default;
};
