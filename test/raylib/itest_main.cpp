#include "core/emulator.hpp"
#include "core/memory.hpp"
#include "core/ppu.hpp"
#include "dummy_types.hpp"
#include "ppu_helper.hpp"
#include "raylib.h"
#include "raylib/cartridge.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

class RaylibPpu final : public CorePpu {
private:
    Color* screenBuffer;

public:
    RaylibPpu( Memory& mem_ ) : CorePpu( mem_ ) {
        screenBuffer =
                static_cast<Color*>( MemAlloc( sizeof( Color ) * displayWidth * displayHeight ) );
        std::memset( screenBuffer, 0, sizeof( Color ) * displayWidth * displayHeight );
    }

    ~RaylibPpu() override {
        if( screenBuffer ) {
            MemFree( screenBuffer );
            screenBuffer = nullptr;
        }
    }

    Color* getScreenBuffer() {
        return screenBuffer;
    }

protected:
    void drawPixel( uint8_t colorId ) override {
        const uint8_t ly = mem.read( Memory::LCD_Y );

        if( ly < displayHeight && state.renderedX < displayWidth ) {
            // Map the GB color ID to a Raylib Color
            const Color pixelColor = { dmgColorMap[colorId][0], dmgColorMap[colorId][1],
                                       dmgColorMap[colorId][2], 255 };

            screenBuffer[ly * displayWidth + state.renderedX] = pixelColor;
        }
    }
};

// Define our emulator type using the Emulator template with our implementations
using LittleBoyEmulator = Emulator<DummyCartridge, Memory, DummyCpu, RaylibPpu>;

int main() {
    // Initialize emulator
    LittleBoyEmulator emu;

    // Setup the display
    const int scaleFactor = 4;
    const int screenWidth = CorePpu::displayWidth * scaleFactor;
    const int screenHeight = CorePpu::displayHeight * scaleFactor;

    InitWindow( screenWidth, screenHeight, "Little boy - Gameboy emulator" );
    SetTargetFPS( 60 );

    // Create a texture to render the Game Boy screen
    Texture2D screenTexture = LoadTextureFromImage(
            GenImageColor( CorePpu::displayWidth, CorePpu::displayHeight, BLACK ) );

    // Initialize test patterns in VRAM
    setupBackgroundChessboardPatternInVram( emu.memory.videoRam );
    setupTestSprites( emu.memory );

    // Setup PPU registers
    emu.memory.write( Memory::LCD_CONTROL, 0x93 ); // Enable LCD, BG and sprites
    emu.memory.write( Memory::LCD_STATUS, 0x00 );  // Mode 0 (H-Blank)
    emu.memory.write( Memory::LCD_Y, 0 );          // LY = 0
    emu.memory.write( Memory::BG_PALETTE, 0xE4 ); // BG palette: black, dark gray, light gray, white
    emu.memory.write( Memory::OBJECT_PALETTE_0, 0xE4 ); // Same palette for sprites
    emu.memory.write( Memory::OBJECT_PALETTE_1, 0x1B ); // Alternate palette for some sprites
    emu.memory.write( Memory::BG_SCROLL_X, 0 );         // No scrolling
    emu.memory.write( Memory::BG_SCROLL_Y, 0 );         // No scrolling

    std::string mousePosition;

    while( !WindowShouldClose() ) {

        // Run PPU for one frame
        for( int ly = 0; ly < CorePpu::displayHeight; ly++ ) {
            emu.memory.write( Memory::LCD_Y, static_cast<uint8_t>( ly ) );

            // Run PPU for this scanline
            for( int i = 0; i < CorePpu::scanlineDuration; i++ ) {
                emu.ppu.tick();
            }
        }

        // Update texture with PPU's output
        UpdateTexture( screenTexture, emu.ppu.getScreenBuffer() );

        BeginDrawing();
        ClearBackground( DARKGRAY );

        // Draw the Game Boy screen scaled to window size
        DrawTexturePro(
                screenTexture,
                Rectangle { 0, 0, (float)CorePpu::displayWidth, (float)CorePpu::displayHeight },
                Rectangle { 0, 0, (float)screenWidth, (float)screenHeight }, Vector2 { 0, 0 }, 0.0f,
                WHITE );

        Vector2 mousePosition = GetMousePosition();
        std::string mousePositionText =
                "X: " + std::to_string( static_cast<int>( mousePosition.x / scaleFactor ) ) +
                ", Y: " + std::to_string( static_cast<int>( mousePosition.y / scaleFactor ) );
        int textWidth = MeasureText( mousePositionText.c_str(), 20 );
        DrawText( mousePositionText.c_str(), screenWidth - textWidth - 10, 10, 20, RED );
        DrawFPS( 5, 5 );
        EndDrawing();
    }

    // Clean up resources
    UnloadTexture( screenTexture );
    CloseWindow();

    return 0;
}
