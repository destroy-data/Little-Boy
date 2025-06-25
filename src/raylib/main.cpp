#include "core/cartridge.hpp"
#include "core/emulator.hpp"
#include "core/logging.hpp"
#include "raylib/raylib_parts.hpp"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <memory>
#include <raylib.h>
#include <utility>
#include <vector>

using Emulator_t            = Emulator<RaylibCpu, RaylibPpu>;
constexpr int targetFps     = 60;
constexpr int ticksPerFrame = ( 1. / targetFps ) * Emulator_t::tickrate;


int main() {
    setLogLevel( LogLevel::Info );
    // clang-format off
    auto romPath = tinyfd_openFileDialog(
        "Choose cartridge",    // dialog title
        "",              // default directory path
        2,                     // number of filter patterns
        CoreCartridge::cartridgeFilePatterns, // filter patterns
        NULL,                  // signle filter description
        0                      // allow multiple selection
    );
    // clang-format on

    std::ifstream romFile( romPath, std::ios::binary );
    if( ! romFile.good() ) {
        logFatal( 0, "Failed to open ROM file: " + std::string( romPath ) );
        return 1;
    }

    auto dataSize = std::filesystem::file_size( romPath );
    std::vector<uint8_t> romData( dataSize );
    romFile.read( reinterpret_cast<char*>( romData.data() ), dataSize );
    romFile.close();

    CoreCartridge::CartridgeType type =
            static_cast<CoreCartridge::CartridgeType>( romData[addr::cartridgeType] );
    std::unique_ptr<CoreCartridge> cartridge { CoreCartridge::create( type, std::move( romData ) ) };

    logDebug( std::format( "Read cartridge type byte: {}", toHex( cartridge->read( addr::cartridgeType ) ) ) );
    logDebug( std::format( "Read ROM size byte: {}", toHex( cartridge->read( addr::romSize ) ) ) );
    logDebug( std::format( "Read RAM size byte: {}", toHex( cartridge->read( addr::ramSize ) ) ) );

    const int scaleFactor  = 7;
    const int screenWidth  = CorePpu::displayWidth * scaleFactor;
    const int screenHeight = CorePpu::displayHeight * scaleFactor;

    InitWindow( screenWidth, screenHeight, "Little boy - Gameboy emulator" );
    SetTargetFPS( 60 );

    Texture2D screenTexture =
            LoadTextureFromImage( GenImageColor( CorePpu::displayWidth, CorePpu::displayHeight, BLACK ) );

    Emulator_t emu( std::move( cartridge ) );
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
            int cycles = 0;
            while( cycles <= ticksPerFrame )
                cycles += emu.tick();
            UpdateTexture( screenTexture, emu.ppu.getScreenBuffer() );
        } else if( doOneTick ) {
            emu.tick();
            UpdateTexture( screenTexture, emu.ppu.getScreenBuffer() );
        }
        doOneTick = false;

        ClearBackground( DARKGRAY );
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

        emu.handleJoypad();
        EndDrawing();
    }

    UnloadTexture( screenTexture );
    CloseWindow();
    return 0;
}
