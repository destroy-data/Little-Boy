#include "raylib/raylib_parts.hpp"
#include "core/memory.hpp"
#include <core/logging.hpp>
#include <cstring>

RaylibPpu::RaylibPpu( Memory& mem_ ) : CorePpu( mem_ ) {
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
    const uint8_t ly = mem.read( addr::lcdY );
    if( ly < displayHeight && state.renderedX < displayWidth ) {
        const Color pixelColor = { dmgColorMap[colorId][0], dmgColorMap[colorId][1], dmgColorMap[colorId][2],
                                   255 };
        screenBuffer[ly * displayWidth + state.renderedX] = pixelColor;
    }
}


//--------------------------------------------------
void RaylibCpu::handleJoypad() {
    const auto joypadInputRegister = mem.read( addr::joypadInput );
    const bool selectButtonsFlag   = joypadInputRegister & ( 1 << 5 );
    const bool selectDPad          = joypadInputRegister & ( 1 << 4 );

    // 0 means button pressed
    uint8_t buttonsPressed = 0x0F;
    if( selectDPad ) {
        if( IsKeyPressed( KEY_S ) )
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 3 ) );
        if( IsKeyPressed( KEY_W ) )
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 2 ) );
        if( IsKeyPressed( KEY_A ) )
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 1 ) );
        if( IsKeyPressed( KEY_D ) )
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 0 ) );
    }
    if( selectButtonsFlag ) {
        if( IsKeyPressed( KEY_Q ) ) // START - TODO: configurable
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 3 ) );
        if( IsKeyPressed( KEY_E ) ) // SELECT - TODO configurable
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 2 ) );
        if( IsKeyPressed( KEY_B ) )
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 1 ) );
        if( IsKeyPressed( KEY_G ) ) // A button - TODO configurable
            mem.write( addr::joypadInput, joypadInputRegister & ~( 1 << 0 ) );
    }

    logDebug( std::format( "Joypad input: {}", buttonsPressed ) );
    mem.write( addr::joypadInput, ( joypadInputRegister & 0xF0 ) | buttonsPressed );
}
