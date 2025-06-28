#pragma once
#include "core/bus.hpp"
#include "core/fetcher.hpp"
#include "core/ppu_types.hpp"
#include <cstdint>
#include <span>

class CorePpu {
public:
    friend class Fetcher;
    static constexpr int displayWidth = 160, displayHeight = 144, tileSize = 16, scanlineDuration = 456;
    using Tilemap_t = std::span<uint8_t, 32 * 32>;
    using Tile_t    = std::span<uint8_t, 16>;
    using TileRow   = std::span<uint8_t, 2>;

    using TileAtlas_t = std::span<uint8_t, 256 * 16>;

    enum class PpuMode { H_BLANK = 0, V_BLANK = 1, OAM_SEARCH = 2, PIXEL_TRANSFER = 3, DISABLED };
    // DMG color values
    const uint8_t dmgColorMap[4][3] = {
            { 255, 255, 255 }, // White
            { 192, 192, 192 }, // Light gray
            { 96, 96, 96 },    // Dark gray
            { 0, 0, 0 }        // Black
    };
    struct {
        SpriteAttribute objects[10] = {};
        unsigned objCount           = 0;
        StaticFifo<Pixel, 8> bgPixelsFifo {};
        StaticFifo<Pixel, 8> spritePixelsFifo {};
        int_fast16_t renderedX = 0;
        int scanlineCycleNr    = 0;
    } state;


    IBus& bus;
    BackgroundFetcher bgFetcher;
    SpriteFetcher spriteFetcher;


    void oamScan();
    uint8_t mergePixel( Pixel bgPixel, Pixel spritePixel );

    virtual void drawPixel( uint8_t colorId ) = 0;

public:
    CorePpu( IBus& bus_ );
    virtual ~CorePpu() = default;
    PpuMode tick();
};
