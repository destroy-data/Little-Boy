#pragma once
#include "core/ppu_types.hpp"
#include <cstdint>

class IBus {
public:
    virtual ~IBus() = default;

    virtual uint8_t read( uint16_t address ) const                           = 0;
    virtual void write( uint16_t address, uint8_t value )                    = 0;
    virtual void setOamLock( bool locked )                                   = 0;
    virtual void setVramLock( bool locked )                                  = 0;
    virtual SpriteAttribute getSpriteAttribute( uint8_t sprite_index ) const = 0;
    virtual uint8_t readVram( uint16_t address ) const                       = 0;
};
