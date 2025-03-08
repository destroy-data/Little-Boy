#pragma once
#include <cstdint>

class ICartridge {
public:
    virtual uint8_t read( const uint16_t address ) = 0;
    virtual void write( const uint16_t address, uint8_t value ) = 0;
    virtual ~ICartridge() = default;
};
