#pragma once
#include "core/fifo.hpp"
#include "core/memory.hpp"
#include <array>
#include <cstdint>
#include <span>

class CorePpu {
public:
    static constexpr int displayWidth = 160, displayHeight = 144, tileSize = 16,
                         scanlineDuration = 456;
    struct Pixel {
        uint8_t colorId : 2;
        uint8_t palette : 3; //in DMG only for objects and only 0 or 1
        uint8_t priority : 1;
        Pixel( uint8_t colorId_ = 0, uint8_t palette_ = 0, uint8_t priority_ = 0 )
            : colorId( colorId_ & 0x3 )
            , palette( palette_ & 0x7 )
            , priority( priority_ & 0x1 ) {
        }
    };
    enum class PpuMode { H_BLANK = 0, V_BLANK = 1, OAM_SEARCH = 2, PIXEL_TRANSFER = 3, DISABLED };
    // DMG color values
    const uint8_t dmgColorMap[4][3] = {
            { 255, 255, 255 }, // White
            { 192, 192, 192 }, // Light gray
            { 96, 96, 96 },    // Dark gray
            { 0, 0, 0 }        // Black
    };
    using TileAtlas_t = std::span<uint8_t, 256 * 16>;
    using Tilemap_t = std::span<uint8_t, 32 * 32>;
    using Tile_t = std::span<uint8_t, 16>;
    Memory& mem;
    // video RAM starts at 0x8000, so adresses must be offseted; Atlases overlap
    TileAtlas_t tileAtlas[2] { TileAtlas_t( mem.videoRam, TileAtlas_t::extent ),
                               TileAtlas_t( mem.videoRam + 0x800, TileAtlas_t::extent ) };
    // tilemaps are in memory at locations 0x9800-0x97FF and 0x9C00-0x9FFF
    Tilemap_t tilemap[2] { Tilemap_t( mem.videoRam + 0x1800, Tilemap_t::extent ),
                           Tilemap_t( mem.videoRam + 0x1C00, Tilemap_t::extent ) };

    struct {
        std::span<uint8_t> object[10] = {};
        unsigned objCount = 0;
        unsigned currentX = 0;
        unsigned renderedX = 0;
        StaticFifo<Pixel, 16> bgPixelsFifo;
        StaticFifo<Pixel, 16> spritePixelsFifo;
        int scanlineCycleNr = 0;
    } state;

    void fetch();
    uint8_t getPixelColor( const Tile_t& tile, int x, int y );
    void oamScan();
    uint8_t mergePixel( Pixel bgPixel, Pixel spritePixel );

    virtual void drawPixel( uint8_t colorId ) = 0;

public:
    CorePpu( Memory& mem_ ) : mem( mem_ ) {
    }
    virtual ~CorePpu() = default;
    PpuMode tick();
};
