#include "core/logging.hpp"
#include "core/ppu.hpp"
#include "dummy_types.hpp"
#include "ppu_helper.hpp"
#include "raylib.h"
#include "raylib/raylib_ppu.hpp"
#include <cstdint>
#include <cstring>

constexpr int targetFps     = 60;
constexpr int ticksPerFrame = int( ( 1. / targetFps ) * constant::tickrate );

void handleJoypad( [[maybe_unused]] IBus& bus ) {
}

int main() {
    std::unique_ptr<DummyCartridge> cartridge = std::make_unique<DummyCartridge>();
    Emulator<RaylibPpu> emu( std::move( cartridge ), handleJoypad );

    const int scaleFactor  = 7;
    const int screenWidth  = CorePpu::displayWidth * scaleFactor;
    const int screenHeight = CorePpu::displayHeight * scaleFactor;

    InitWindow( screenWidth, screenHeight, "Little boy - Gameboy emulator" );
    SetTargetFPS( 60 );

    Texture2D screenTexture =
            LoadTextureFromImage( GenImageColor( CorePpu::displayWidth, CorePpu::displayHeight, BLACK ) );

    setupBackgroundChessboardPatternInVram( emu );
    setupLcdRegisters( emu );

    bool interactiveDebugMode = true;
    bool emulationStopped     = false;
    bool doOneTick; // When emulation isn't stopped, the value doesn't matter
    while( ! WindowShouldClose() ) {
        if( IsKeyPressed( KEY_C ) ) {
            interactiveDebugMode = ! interactiveDebugMode;
            emulationStopped     = false;
        }
        if( interactiveDebugMode && IsKeyPressed( KEY_H ) ) {
            emulationStopped = ! emulationStopped;
            if( emulationStopped )
                logLiveDebug( "Stopped emulation!" );
            else
                logLiveDebug( "Start emulation again!" );
        }
        if( interactiveDebugMode && IsKeyPressed( KEY_U ) )
            logSeparator();
        if( interactiveDebugMode && IsKeyPressed( KEY_J ) )
            doOneTick = true;
        if( IsKeyPressed( KEY_KP_ADD ) ) {
            setLogLevel( LogLevel( std::max( std::to_underlying( getLogLevel() ) - 1, 0 ) ) );
            logLiveDebug( std::format( "Log level decreased to {}", std::to_underlying( getLogLevel() ) ) );
        }
        if( IsKeyPressed( KEY_KP_SUBTRACT ) ) {
            setLogLevel( LogLevel( std::min( std::to_underlying( getLogLevel() ) + 1,
                                             int( std::numeric_limits<uint8_t>::max() ) ) ) );
            logLiveDebug( std::format( "Log level increased to {}", std::to_underlying( getLogLevel() ) ) );
        }

        BeginDrawing();
        if( ! emulationStopped ) {
            for( int i = 0; i <= ticksPerFrame; i++ )
                emu.ppu.tick();
        } else if( doOneTick ) {
            emu.ppu.tick();
        }
        doOneTick = false;

        ClearBackground( DARKGRAY );
        UpdateTexture( screenTexture, emu.ppu.getScreenBuffer() );
        DrawTexturePro(
                screenTexture, Rectangle { 0, 0, (float)CorePpu::displayWidth, (float)CorePpu::displayHeight },
                Rectangle { 0, 0, (float)screenWidth, (float)screenHeight }, Vector2 { 0, 0 }, 0.0f, WHITE );


        if( interactiveDebugMode ) {
            Vector2 mousePosition = GetMousePosition();
            std::string mousePositionText =
                    "X: " + std::to_string( static_cast<int>( mousePosition.x / scaleFactor ) ) +
                    ", Y: " + std::to_string( static_cast<int>( mousePosition.y / scaleFactor ) );
            int textWidth = MeasureText( mousePositionText.c_str(), 20 );
            DrawText( mousePositionText.c_str(), screenWidth - textWidth - 10, 10, 20, RED );
            DrawFPS( 5, 5 );
        }
        EndDrawing();
    }

    UnloadTexture( screenTexture );
    CloseWindow();
    return 0;
}
