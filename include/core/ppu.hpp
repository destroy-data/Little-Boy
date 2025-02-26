#pragma once
#include "core/memory.hpp"
#include <cstdint>
#include <span>

class PPU {
    using Tile = std::span<uint8_t, 16>;
    using TileAtlas = std::span<uint8_t, 2048>; // 128 tiles * 16 bytes
    using Tilemap = std::span<uint8_t, 1024>;
    Memory& mem;
    TileAtlas tileAtlas[3];
    Tilemap tilemap[2];

    uint8_t& getLCDC() {
        return mem[0xFF40];
    }

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
