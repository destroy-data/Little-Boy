#include "core/cpu.hpp"
#include "core/memory.hpp"
#include "raylib.h"
#include <cstdint>
#include <cstdio>

int main() {
    Memory mem;
    CPU cpu( mem );

    InitWindow( 160 * 7, 144 * 7, "Little boy" );
    SetTargetFPS( 60 ); //It's the closest to 59.7 Gameboy had

    while( !WindowShouldClose() ) {
        BeginDrawing();
        ClearBackground( DARKGRAY );
        EndDrawing();

        cpu.handleJoypad();
    }

    CloseWindow();
    return 0;
}

void CPU::handleJoypad() {
    //TODO make buttons configurable
    auto& joyp = mem[Memory::JOYPAD_INPUT];
    uint8_t result = 0x0F; // only first half-byte used, 0 means pressed
    // bit 0 is A/Right, bit 1 is B/Left, bit 2 is Select/Up, bit 3 is Start/Down

    //If both D-pad and buttons are selected, write 0 if either is pressed
    if( !( joyp & ( 1 << 4 ) ) ) { //Select D-pad
        if( IsKeyDown( KEY_D ) )
            result &= static_cast<uint8_t>( ~1 );
        if( IsKeyDown( KEY_A ) )
            result &= static_cast<uint8_t>( ~( 1 << 1 ) );
        if( IsKeyDown( KEY_W ) )
            result &= static_cast<uint8_t>( ~( 1 << 2 ) );
        if( IsKeyDown( KEY_S ) )
            result &= static_cast<uint8_t>( ~( 1 << 3 ) );
    }
    if( !( joyp & ( 1 << 5 ) ) ) { //Select buttons
        if( IsKeyDown( KEY_Q ) )
            result &= static_cast<uint8_t>( ~1 );
        if( IsKeyDown( KEY_E ) )
            result &= static_cast<uint8_t>( ~( 1 << 1 ) );
        if( IsKeyDown( KEY_R ) )
            result &= static_cast<uint8_t>( ~( 1 << 2 ) );
        if( IsKeyDown( KEY_T ) )
            result &= static_cast<uint8_t>( ~( 1 << 3 ) );
    }

    if( joyp & ( result ^ ( joyp & 0x0F ) ) )      // at lest one bit was 1 and changed to 0
        mem[Memory::INTERRUPT_FLAG] |= ( 1 << 4 ); // so set joypad interrupt flag
    joyp = ( joyp & 0xF0 ) | result;
}
