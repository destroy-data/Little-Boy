#pragma once
#include "core/fifo.hpp"
#include "core/memory.hpp"
#include <array>
#include <cstdint>
#include <span>

class CorePpu {
public:
    friend class Fetcher;
    static constexpr int displayWidth = 160, displayHeight = 144, tileSize = 16,
                         scanlineDuration = 456;
    struct Pixel {
        uint8_t colorId : 2;
        uint8_t palette : 3; //in DMG only for objects and only 0 or 1
        uint8_t bgPriority : 1;
        Pixel( uint8_t colorId_ = 0, uint8_t palette_ = 0, uint8_t bgPriority_ = 0 )
            : colorId( colorId_ & 0x3 )
            , palette( palette_ & 0x7 )
            , bgPriority( bgPriority_ & 0x1 ) {
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
    using TileRow = std::span<uint8_t, 2>;

    class BackgroundFetcher {
    public:
        enum class FetcherState_t { FETCH_TILE, FETCH_DATA_LOW, FETCH_DATA_HIGH, PUSH };
        FetcherState_t state = FetcherState_t::FETCH_TILE;
        uint_fast8_t ticksInCurrentState;
        uint8_t currentTileX;
        uint8_t tileId;
        uint8_t tileDataLow;
        uint8_t tileDataHigh;
        CorePpu& ppu;
        Memory& mem;

        BackgroundFetcher( CorePpu& ppu_, Memory& mem_ ) : ppu( ppu_ ), mem( mem_ ) {
        }
        void tick();
    };

    Memory& mem;
    // video RAM starts at 0x8000, so adresses must be offseted; Atlases overlap
    TileAtlas_t tileAtlas[2] { TileAtlas_t( mem.videoRam, TileAtlas_t::extent ),
                               TileAtlas_t( mem.videoRam + 0x800, TileAtlas_t::extent ) };
    // tilemaps are in memory at locations 0x9800-0x97FF and 0x9C00-0x9FFF
    Tilemap_t tilemap[2] { Tilemap_t( mem.videoRam + 0x1800, Tilemap_t::extent ),
                           Tilemap_t( mem.videoRam + 0x1C00, Tilemap_t::extent ) };

    BackgroundFetcher bgFetcher;
    struct {
        std::span<uint8_t> object[10] = {};
        unsigned objCount = 0;
        StaticFifo<Pixel, 8> bgPixelsFifo {};
        StaticFifo<Pixel, 8> spritePixelsFifo {};
        int_fast16_t renderedX;
        int scanlineCycleNr = 0;
    } state;

    void oamScan();
    uint8_t mergePixel( Pixel bgPixel, Pixel spritePixel );

    virtual void drawPixel( uint8_t colorId ) = 0;

public:
    CorePpu( Memory& mem_ ) : mem( mem_ ), fetcher { *this, mem_ } {
    }
    virtual ~CorePpu() = default;
    PpuMode tick();
};
