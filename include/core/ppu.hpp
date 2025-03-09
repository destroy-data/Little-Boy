#pragma once
#include "core/memory.hpp"
#include <cstdint>
#include <span>

class PPU {
    static constexpr int displayWidth = 160, displayHeight = 144, tileSize = 16;
    enum registers {
        LCD_CONTROL = 0xFF40,
        LCD_STATUS = 0xFF41,
        BG_SCROLL_Y = 0xFF42,
        BG_SCROLL_X = 0xFF43,
        LCD_Y = 0xFF44,
        LYC = 0xFF45,
        // non-CGB mode only
        BG_PALETTE = 0xFF47,
        OBJECT_PALETTE_0 = 0xFF48,
        OBJECT_PALETTE_1 = 0xFF49,
        // both modes
        WIN_Y = 0xFF4A,
        WIN_X = 0xFF4B,
        // CGB mode only
        BG_PALETTE_SPEC = 0xFF68,
        BG_PALETTE_DATA = 0xFF69,
        OBJ_PALETTE_SPEC = 0xFF6A,
        OBJ_PALETTE_DATA = 0xFF6B
    };
    // DMG color values
    const uint8_t colorMap[4][3] = {
            { 255, 255, 255 }, // White
            { 192, 192, 192 }, // Light gray
            { 96, 96, 96 },    // Dark gray
            { 0, 0, 0 }        // Black
    };
    using TileAtlas_t = std::span<uint8_t, 256 * 16>;
    using Tilemap_t = std::span<uint8_t, 32 * 32>;
    using Tile_t = std::span<uint8_t, 16>;
    Memory& mem;
    TileAtlas_t tileAtlas[2]; // they overlap
    Tilemap_t tilemap[2];

    uint8_t getPixelColor( Tile_t tile, int x, int y );

public:
    PPU( Memory& mem_ )
        : mem( mem_ )
        // video RAM starts at 0x8000, so adresses must be offseted
        , tileAtlas { TileAtlas_t( mem.videoRam, TileAtlas_t::extent ),
                      TileAtlas_t( mem.videoRam + 0x800, TileAtlas_t::extent ) }
        // tilemaps are in memory at locations 0x9800-0x97FF and 0x9C00-0x9FFF
        , tilemap { Tilemap_t( mem.videoRam + 0x1800, Tilemap_t::extent ),
                    Tilemap_t( mem.videoRam + 0x1C00, Tilemap_t::extent ) } {
    }

    std::array<uint8_t, displayWidth * 3> draw();
};
