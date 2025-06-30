#pragma once
#include "core/bus.hpp"
#include "core/core_constants.hpp"
#include <raylib.h>

void raylibHandleJoypad( IBus& bus ) {
    const auto joypadInputRegister = bus.read( addr::joypadInput );
    const bool selectButtonsFlag   = joypadInputRegister & ( 1 << 5 );
    const bool selectDPad          = joypadInputRegister & ( 1 << 4 );

    // 0 means button pressed
    uint8_t buttonsPressed = 0x0F;
    if( selectDPad ) {
        if( IsKeyPressedRepeat( KEY_S ) )
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 3 ) );
        if( IsKeyPressedRepeat( KEY_W ) )
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 2 ) );
        if( IsKeyPressedRepeat( KEY_A ) )
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 1 ) );
        if( IsKeyPressedRepeat( KEY_D ) )
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 0 ) );
    }
    if( selectButtonsFlag ) {
        if( IsKeyPressedRepeat( KEY_Q ) ) // START - TODO: configurable
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 3 ) );
        if( IsKeyPressedRepeat( KEY_E ) ) // SELECT - TODO configurable
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 2 ) );
        if( IsKeyPressedRepeat( KEY_B ) )
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 1 ) );
        if( IsKeyPressedRepeat( KEY_G ) ) // A button - TODO configurable
            bus.write( addr::joypadInput, joypadInputRegister & ~( 1 << 0 ) );
    }

    bus.write( addr::joypadInput, ( joypadInputRegister & 0xF0 ) | buttonsPressed );
}
