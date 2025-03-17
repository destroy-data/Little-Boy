#pragma once
#include "core/fifo.hpp"
#include "core/memory.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <span>

class PPU {
public:
    friend class Tester;
    static constexpr int displayWidth = 160, displayHeight = 144, tileSize = 16,
                         scanlineDuration = 456;
    struct Pixel {
        unsigned color : 2;
        unsigned palette : 3;  //in DMG only for objects and only 0 or 1
        unsigned priority : 1; //CGB only
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
    uint64_t& tickNr;
    Memory& mem;
    TileAtlas_t tileAtlas[2]; // they overlap
    Tilemap_t tilemap[2];

    struct {
        std::span<uint8_t> object[10] = {};
        int objCount = 0;
        int currentX = 0;
        StaticFifo<Pixel, 16> bgPixelsFifo;
        StaticFifo<Pixel, 16> spritePixelsFifo;
        int scanlineCycleNr = 0; // one scanline takes 456 cycles
        std::optional<Pixel> toDraw;
    } state;

    std::array<Pixel, 8> fetch();
    uint8_t getPixelColor( const Tile_t& tile, int x, int y );
    void oamScan();
    uint8_t mergePixel( Pixel bgPixel, Pixel spritePixel );

    // Functions not implemented in core
    void drawPixel();

public:
    PPU( uint64_t& tickNr_, Memory& mem_ )
        : tickNr( tickNr_ )
        , mem( mem_ )
        // video RAM starts at 0x8000, so adresses must be offseted
        , tileAtlas { TileAtlas_t( mem.videoRam, TileAtlas_t::extent ),
                      TileAtlas_t( mem.videoRam + 0x800, TileAtlas_t::extent ) }
        // tilemaps are in memory at locations 0x9800-0x97FF and 0x9C00-0x9FFF
        , tilemap { Tilemap_t( mem.videoRam + 0x1800, Tilemap_t::extent ),
                    Tilemap_t( mem.videoRam + 0x1C00, Tilemap_t::extent ) } {
    }

    void tick();
};
