#include "raylib/raylib_ppu.hpp"
#include "core/core_constants.hpp"
#include <core/logging.hpp>
#include <cstring>

RaylibPpu::RaylibPpu( IBus& bus_ ) : CorePpu( bus_ ) {
    screenBuffer = static_cast<Color*>( MemAlloc( sizeof( Color ) * displayWidth * displayHeight ) );
    std::memset( screenBuffer, 0, sizeof( Color ) * displayWidth * displayHeight );
}

RaylibPpu::~RaylibPpu() {
    if( screenBuffer ) {
        MemFree( screenBuffer );
        screenBuffer = nullptr;
    }
}

Color* RaylibPpu::getScreenBuffer() {
    return screenBuffer;
}

void RaylibPpu::drawPixel( uint8_t colorId ) {
    const uint8_t ly = bus.read( addr::lcdY );
    if( ly < displayHeight && state.renderedX < displayWidth ) {
        const Color pixelColor = { dmgColorMap[colorId][0], dmgColorMap[colorId][1], dmgColorMap[colorId][2],
                                   255 };
        screenBuffer[ly * displayWidth + state.renderedX] = pixelColor;
    }
}
