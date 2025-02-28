#pragma once
#include "core/memory.hpp"
#include <cstdint>
#include <span>

class PPU {
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
    using Tile = std::span<uint8_t, 16>;
    using TileAtlas = std::span<uint8_t, 2048>; // 128 tiles * 16 bytes
    using Tilemap = std::span<uint8_t, 1024>;
    Memory& mem;
    TileAtlas tileAtlas[3];
    Tilemap tilemap[2];

public:
    PPU( Memory& mem_ )
        : mem( mem_ )
        // video RAM starts at 0x8000, so adresses must be offseted
        , tileAtlas { TileAtlas( mem.videoRam, TileAtlas::extent ),
                      TileAtlas( mem.videoRam + 0x800, TileAtlas::extent ),
                      TileAtlas( mem.videoRam + 0x1000, TileAtlas::extent ) }
        // tilemaps are in memory at locations 0x9800-0x97FF and 0x9C00-0x9FFF
        , tilemap { Tilemap( mem.videoRam + 0x1800, Tilemap::extent ),
                    Tilemap( mem.videoRam + 0x1C00, Tilemap::extent ) } {
    }
};
