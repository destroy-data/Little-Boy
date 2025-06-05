#include "core/memory.hpp"
#include "core/ppu.hpp"
#include "dummy_types.hpp"
#include "ppu_helper.hpp"
#include "raylib.h"
#include <cstdint>
#include <cstring>

class RaylibPpu final : public CorePpu {
private:
    Color* screenBuffer;

public:
    RaylibPpu( Memory& mem_ ) : CorePpu( mem_ ) {
        screenBuffer = static_cast<Color*>( MemAlloc( sizeof( Color ) * displayWidth * displayHeight ) );
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
        const uint8_t ly = mem.read( addr::lcdY );
        if( ly < displayHeight && state.renderedX < displayWidth ) {
            const Color pixelColor = { dmgColorMap[colorId][0], dmgColorMap[colorId][1],
                                       dmgColorMap[colorId][2], 255 };
            screenBuffer[ly * displayWidth + state.renderedX] = pixelColor;
        }
    }
};

using RaylibEmulator = Emulator<DummyCpu, RaylibPpu>;

int main() {
    std::unique_ptr<DummyCartridge> cartridge = std::make_unique<DummyCartridge>();
    RaylibEmulator emu( std::move( cartridge ) );

    const int scaleFactor  = 7;
    const int screenWidth  = CorePpu::displayWidth * scaleFactor;
    const int screenHeight = CorePpu::displayHeight * scaleFactor;

    InitWindow( screenWidth, screenHeight, "Little boy - Gameboy emulator" );
    SetTargetFPS( 60 );

    Texture2D screenTexture =
            LoadTextureFromImage( GenImageColor( CorePpu::displayWidth, CorePpu::displayHeight, BLACK ) );

    setupBackgroundChessboardPatternInVram( emu.memory.videoRam );
    setupLcdRegisters( emu.memory );

    while( !WindowShouldClose() ) {
        for( int ly = 0; ly < CorePpu::displayHeight; ly++ ) {
            for( int i = 0; i < CorePpu::scanlineDuration; i++ ) {
                emu.ppu.tick();
            }
        }

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
