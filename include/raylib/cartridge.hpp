#pragma once
#include "core/cartridge.hpp"
#include <cstdint>
#include <span>
#include <vector>

class RaylibCartridge final : public CoreCartridge {
    static constexpr auto romExtensions = { ".gb", ".sgb" };
    std::vector<uint8_t> rom;
    std::span<uint8_t> romBank0N;

public:
    RaylibCartridge();
    ~RaylibCartridge() final = default;
    virtual uint8_t read( const uint16_t address );
    virtual void write( const uint16_t address, uint8_t value );
};
