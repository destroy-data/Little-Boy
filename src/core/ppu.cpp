#include "core/core_constants.hpp"
#include "core/logging.hpp"
#include <core/ppu.hpp>
#include <cstdint>
#include <utility>

void CorePpu::oamScan() {
    //mode 2 - search for objects which overlap current scanline
    //it takes 80 dots
    state.objCount        = 0;
    const bool objSize8x8 = ~bus.read( addr::lcdControl ) & ( 1 << 2 );
    const int ly          = bus.read( addr::lcdY );

    for( uint8_t i = 0; i < 40 && state.objCount < 10; i++ ) {
        const auto sprite = bus.getSpriteAttribute( i );
        if( ly >= sprite.y - 16 && ly < sprite.y - 8 * objSize8x8 ) {
            state.objects[state.objCount++] = sprite;
        }
    }

    // in non-CGB mode draw priority differs from selection priority, so sort the array
    // it's x position based, lower x is higher priority
    // if Xs are equal, first one in OAM has higher priority, fortunately insertion sort is stable
    for( unsigned i = 1; i < state.objCount; i++ ) {
        const auto key = state.objects[i];
        int j          = static_cast<int>( i - 1 );
        while( j >= 0 && state.objects[j].x > key.x ) {
            state.objects[j + 1] = state.objects[j];
            j--;
        }
        state.objects[j + 1] = key;
    }
}

CorePpu::PpuMode CorePpu::tick() {
    // Check if LCD is enabled
    const uint8_t lcdc = bus.read( addr::lcdControl );
    if( ! ( lcdc & ( 1 << 7 ) ) ) {
        return PpuMode::DISABLED; // LCD disabled, nothing to do
    }

    uint8_t status            = bus.read( addr::lcdStatus );
    const auto currentMode    = static_cast<PpuMode>( status & 0x3 );
    uint8_t ly                = bus.read( addr::lcdY );
    auto newLy                = ly;
    bool resetScanlineCycleNr = false;

    // State machine to handle PPU modes
    logDebug( std::format( "PPU mode<{}>", int( currentMode ) ) );
    switch( currentMode ) {
        using enum PpuMode;
    case H_BLANK:
        if( state.scanlineCycleNr >= scanlineDuration - 1 ) {
            resetScanlineCycleNr = true;
            newLy++;

            if( ly >= 144 ) {
                status = ( status & ~0x3 ) | static_cast<uint8_t>( V_BLANK );
                bus.write( addr::lcdStatus, status );
                bus.setOamLock( false );

                // Request V-Blank interrupt
                bus.write( addr::interruptFlag, bus.read( addr::interruptFlag ) | bitMask::vBlankInterrupt );
            } else {
                status = ( status & ~0x3 ) | static_cast<uint8_t>( OAM_SEARCH );
                bus.write( addr::lcdStatus, status );
                bus.setOamLock( true );
            }
        }
        break;

    case V_BLANK:
        if( state.scanlineCycleNr >= scanlineDuration - 1 ) {
            resetScanlineCycleNr = true;

            if( ly >= 153 ) {
                // End of V-Blank, back to first scanline
                newLy = 0;

                status = ( status & ~0x3 ) | static_cast<uint8_t>( OAM_SEARCH );
                bus.write( addr::lcdStatus, status );
                bus.setOamLock( true );
            } else
                newLy++;
        }
        break;

    case OAM_SEARCH:
        if( state.scanlineCycleNr >= 80 ) { // OAM search lasts 80 cycles
            // At least for now do it in one go
            oamScan();

            status = ( status & ~0x3 ) | static_cast<uint8_t>( PIXEL_TRANSFER );
            bus.write( addr::lcdStatus, status );
        }
        break;

    case PIXEL_TRANSFER:
        bgFetcher.tick();
        if( ! state.bgPixelsFifo.empty() && state.renderedX < displayWidth ) {
            // Get and mix pixels from both FIFOs (for now, sprite FIFO will be empty)
            const Pixel bgPixel = state.bgPixelsFifo.pop();
            const Pixel spritePixel =
                    state.spritePixelsFifo.empty() ? Pixel( 0, 0, 0 ) : state.spritePixelsFifo.pop();

            drawPixel( mergePixel( bgPixel, spritePixel ) );
            state.renderedX++;
        }

        if( state.renderedX >= displayWidth ) {
            // Move to H-Blank
            state.renderedX = 0;
            status          = ( status & ~0x3 ) | static_cast<uint8_t>( H_BLANK );
            bus.write( addr::lcdStatus, status );
            bus.setVramLock( false );
            bus.setOamLock( false );

            state.bgPixelsFifo.clear();
            state.spritePixelsFifo.clear();
            bgFetcher.reset();
            spriteFetcher.reset();
        }
        break;
    case DISABLED:
        std::unreachable();
    }
    if( resetScanlineCycleNr )
        state.scanlineCycleNr = 0;
    else
        state.scanlineCycleNr++;
    bus.write( addr::lcdY, newLy );
    return static_cast<PpuMode>( status & 0x3 );
}

uint8_t CorePpu::mergePixel( Pixel bgPixel, Pixel spritePixel ) {
    // Merge background and object pixels
    const uint8_t lcdc    = bus.read( addr::lcdControl );
    const bool bgEnabled  = lcdc & 0x01;
    const bool objEnabled = lcdc & 0x02;

    const uint8_t bgPalette   = bus.read( addr::bgPalette );
    const uint8_t objPalette0 = bus.read( addr::objectPalette0 );
    const uint8_t objPalette1 = bus.read( addr::objectPalette1 );

    uint8_t finalColor;
    // Determine which pixel to display according to priority rules
    if( objEnabled && spritePixel.colorId != 0 ) {
        // Sprite pixel is not transparent
        if( bgEnabled && bgPixel.colorId != 0 && spritePixel.bgPriority ) {
            // Background has priority over this sprite and is not transparent
            finalColor = bgPalette >> ( bgPixel.colorId * 2 ) & 0x03;
        } else {
            // Sprite has priority or background is transparent/disabled
            const uint8_t objPalette = spritePixel.palette ? objPalette1 : objPalette0;
            finalColor               = objPalette >> ( spritePixel.colorId * 2 ) & 0x03;
        }
    } else if( bgEnabled ) {
        finalColor = bgPalette >> ( bgPixel.colorId * 2 ) & 0x03;
    } else {
        finalColor = 0;
    }
    return finalColor;
}

CorePpu::CorePpu( IBus& bus_ )
    : bus( bus_ )
    , bgFetcher { bus_, this->state.bgPixelsFifo }
    , spriteFetcher( bus_, this->state.spritePixelsFifo ) {
    uint8_t status = bus.read( addr::lcdStatus );
    status         = ( status & ~0x3 ) | static_cast<uint8_t>( PpuMode::OAM_SEARCH );
    bus.write( addr::lcdStatus, status );
}
