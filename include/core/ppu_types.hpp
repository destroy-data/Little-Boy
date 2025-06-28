#pragma once
#include "core/fifo.hpp"
#include <cstdint>

struct SpriteAttribute {
    uint8_t y;
    uint8_t x;
    uint8_t tileIndex;
    uint8_t flags;
};

struct Pixel {
    uint8_t colorId : 2;
    uint8_t palette : 3; //in DMG only for objects and only 0 or 1
    uint8_t bgPriority : 1;
    Pixel( uint8_t colorId_ = 0, uint8_t palette_ = 0, uint8_t bgPriority_ = 0 )
        : colorId( colorId_ & 0x3 )
        , palette( palette_ & 0x7 )
        , bgPriority( bgPriority_ & 0x1 ) {
    }
};

using PixelFifo = StaticFifo<Pixel, 8>;
