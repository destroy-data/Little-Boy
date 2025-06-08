#include "core/cartridge.hpp"
#include "core/emulator.hpp"
#include "core/logging.hpp"
#include "raylib/raylib_parts.hpp"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <raylib.h>
#include <vector>

using Emulator_t = Emulator<RaylibCpu, RaylibPpu>;

int main() {
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
    if( !romFile.good() ) {
        logFatal( 0, "Failed to open ROM file: " + std::string( romPath ) );
        return 1;
    }

    auto dataSize = std::filesystem::file_size( romPath );
    std::vector<uint8_t> romData( dataSize );
    romFile.read( reinterpret_cast<char*>( romData.data() ), dataSize );
    romFile.close();


    CoreCartridge::CartridgeType type = static_cast<CoreCartridge::CartridgeType>( romData[addr::romSize] );
    std::unique_ptr<CoreCartridge> cartridge { CoreCartridge::create( type, std::move( romData ) ) };

    logDebug( std::format( "Read cartridge type byte: {}", toHex( cartridge->read( addr::cartridgeType ) ) ) );
    logDebug( std::format( "Read ROM size byte: {}", toHex( cartridge->read( addr::romSize ) ) ) );
    logDebug( std::format( "Read RAM size byte: {}", toHex( cartridge->read( addr::ramSize ) ) ) );

    logDebug( std::format( "Try to read from ROM bank 1. Read from address {}: {}",
                           toHex( uint16_t { 0x4000 } ), toHex( cartridge->read( 0x4000 ) ) ) );

    logDebug( std::format( "Try to read from RAM (which does not exist). Read from address {}: {}",
                           toHex( uint16_t { 0xA000 } ), toHex( cartridge->read( 0xA000 ) ) ) );

    const int scaleFactor  = 7;
    const int screenWidth  = CorePpu::displayWidth * scaleFactor;
    const int screenHeight = CorePpu::displayHeight * scaleFactor;

    InitWindow( screenWidth, screenHeight, "Little boy - Gameboy emulator" );
    SetTargetFPS( 60 );

    Texture2D screenTexture =
            LoadTextureFromImage( GenImageColor( CorePpu::displayWidth, CorePpu::displayHeight, BLACK ) );

    Emulator_t emu( std::move( cartridge ) );
    while( !WindowShouldClose() ) {
        emu.tick();
        UpdateTexture( screenTexture, emu.ppu.getScreenBuffer() );

        BeginDrawing();
        ClearBackground( DARKGRAY );

        DrawTexturePro(
                screenTexture, Rectangle { 0, 0, (float)CorePpu::displayWidth, (float)CorePpu::displayHeight },
                Rectangle { 0, 0, (float)screenWidth, (float)screenHeight }, Vector2 { 0, 0 }, 0.0f, WHITE );

        Vector2 mousePosition = GetMousePosition();
        std::string mousePositionText =
                "X: " + std::to_string( static_cast<int>( mousePosition.x / scaleFactor ) ) +
                ", Y: " + std::to_string( static_cast<int>( mousePosition.y / scaleFactor ) );
        int textWidth = MeasureText( mousePositionText.c_str(), 20 );
        DrawText( mousePositionText.c_str(), screenWidth - textWidth - 10, 10, 20, RED );
        DrawFPS( 5, 5 );

        EndDrawing();
    }

    UnloadTexture( screenTexture );
    CloseWindow();
    return 0;
}
