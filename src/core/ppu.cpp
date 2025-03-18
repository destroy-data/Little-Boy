#include "core/memory.hpp"
#include <core/ppu.hpp>
#include <cstdint>

void PPU::oamScan() {
    //mode 2 - search for objects which overlap current scanline
    //it takes 80 dots
    state.objCount = 0;
    const bool objSize8x8 = ~mem.read( Memory::LCD_CONTROL ) & ( 1 << 2 );
    const int ly = mem.read( Memory::LCD_Y );

    // obj Y = pos + 16
    for( unsigned i = 0; i < sizeof( mem.oam ) && state.objCount < 10; i += 4 ) {
        if( ly >= mem.oam[i] - 16 && ly < mem.oam[i] - 8 * objSize8x8 ) {
            state.object[state.objCount++] = { &mem.oam[i], 4 }; //0-Y pos, 1-X pos
        }
    }

    // in non-CGB mode draw priority differs from selection priority, so sort the array
    // it's x position based, lower x is higher priority
    // if Xs are equal, first one in OAM has higher priority, fortunately insertion sort is stable
    for( unsigned i = 1; i < state.objCount; i++ ) {
        const auto key = state.object[i];
        int j = static_cast<int>( i - 1 );
        while( j >= 0 && state.object[j][1] > key[1] ) {
            state.object[j + 1] = state.object[j];
            j--;
        }
        state.object[j + 1] = key;
    }
}

std::array<PPU::Pixel, 8> PPU::fetch() {
    const uint8_t lcdc = mem.read( Memory::LCD_CONTROL );
    const bool bgWinEnabled = lcdc & 0x1;
    const uint8_t ly = mem.read( Memory::LCD_Y );
    const uint8_t winX = mem.read( Memory::WIN_X );
    const uint8_t winY = mem.read( Memory::WIN_Y );
    const uint8_t scrollX = mem.read( Memory::BG_SCROLL_X );
    const uint8_t scrollY = mem.read( Memory::BG_SCROLL_Y );
    const bool winEnabled = ( lcdc & ( 1 << 5 ) ) && ( winY <= ly ) && ( winX < 168 );
    const bool objEnabled = lcdc & ( 1 << 1 );
    const bool useSecondBgMap = lcdc & ( 1 << 3 );
    const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
    const bool useSecondWinMap = lcdc & ( 1 << 6 );

    std::array<Pixel, 8> bgBuffer {};
    std::array<Pixel, 8> objectBuffer {};
    const unsigned end = state.currentX + 8;
    for( unsigned i = 0; state.currentX < end; ++i, ++state.currentX ) {
        if( bgWinEnabled ) {
            const bool windowPixel =
                    winEnabled && ( state.currentX >= static_cast<uint8_t>( winX - 7 ) );
            uint8_t colorId = 0;
            if( windowPixel ) {
                const auto pixX = static_cast<uint8_t>( state.currentX - winX + 7 );
                const auto pixY = static_cast<uint8_t>( ly - winY );
                const uint8_t winTileId = tilemap[useSecondWinMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress = // 0 is start of VRAM
                        base8000Addr ? tileSize * winTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( winTileId );
                const auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
                bgBuffer[i] = { .colorId = colorId & 0x3u };
            } else {
                const auto pixX = static_cast<uint8_t>( state.currentX + scrollX );
                const auto pixY = static_cast<uint8_t>( ly + scrollY );
                const auto bgTileId = tilemap[useSecondBgMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress = // 0 is start of VRAM
                        base8000Addr ? tileSize * bgTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( bgTileId );
                const auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
                bgBuffer[i] = { .colorId = colorId & 0x3u };
            }
        }
        if( objEnabled ) {
            const int objHeight = ( ~lcdc & ( 1 << 2 ) ) ? 8 : 16;

            // For each object found during OAM scan
            for( unsigned j = 0; j < state.objCount; j++ ) {
                const uint8_t objX = state.object[j][1] - 8;
                const uint8_t objY = state.object[j][0] - 16;

                if( objY + objHeight < ly || objY > ly )
                    continue;
                if( static_cast<unsigned>( objX + 8 ) <= state.currentX || objX >= end )
                    continue;

                uint8_t tileId = state.object[j][2];
                if( objHeight == 16 )
                    tileId &= 0xFE;

                const uint8_t attributes = state.object[j][3];
                const bool xFlip = attributes & ( 1 << 5 );
                const bool yFlip = attributes & ( 1 << 6 );
                const bool priority = attributes & ( 1 << 7 );
                const bool dmgUseOBP1 = attributes & ( 1 << 4 );

                uint8_t tileY = static_cast<uint8_t>( ly - objY );
                if( yFlip ) {
                    uint8_t flip = static_cast<uint8_t>( objHeight - 1 );
                    tileY = static_cast<uint8_t>( flip - tileY );
                }

                // objects always use 8000 addressing mode
                const auto tile =
                        Tile_t( &mem.videoRam[tileId * ( tileY < 8 ? tileSize : tileSize + 1 )],
                                Tile_t::extent );

                if( tileY >= 8 )
                    tileY -= 8;

                for( unsigned x = state.currentX; x < end && x < static_cast<unsigned>( objX + 8 );
                     x++ ) { //FIXME
                    // Skip if pixel is outside object bounds
                    if( x < objX || x >= static_cast<unsigned>( objX + 8 ) )
                        continue;

                    unsigned fifoIndex = x - state.currentX;
                    uint8_t tileX = static_cast<uint8_t>( x - objX );
                    if( xFlip ) {
                        tileX = 7 - tileX;
                    }
                    uint8_t colorId = getPixelColor( tile, tileX, tileY );

                    // Color 0 is transparent for objects
                    if( objectBuffer[fifoIndex].colorId == 0 ) {
                        objectBuffer[fifoIndex] = { .colorId = colorId & 0x3u,
                                                    .palette = dmgUseOBP1,
                                                    .priority = priority };
                    }
                }
            }
        }
    }
    state.bgPixelsFifo.pushBatch( bgBuffer );
    return bgBuffer;
}

uint8_t PPU::getPixelColor( const Tile_t& tile, int x, int y ) {
    if( x < 0 || x > 7 || y < 0 || y > 7 ) {
        return 0; // no better idea
    }
    // How tiles are encoded:
    // https://gbdev.io/pandocs/Tile_Data.html#data-format
    int bitPosition = 7 - x;
    auto rowIndex = static_cast<std::size_t>( y * 2 );
    bool lBit = ( tile[rowIndex] & ( 1 << bitPosition ) );
    bool hBit = ( tile[rowIndex + 1] & ( 1 << bitPosition ) );

    return static_cast<uint8_t>( ( hBit << 1 ) | lBit );
}


void PPU::tick() {
    // Check if LCD is enabled
    const uint8_t lcdc = mem.read( Memory::LCD_CONTROL );
    if( !( lcdc & ( 1 << 7 ) ) ) {
        return; // LCD disabled, nothing to do
    }

    uint8_t status = mem.read( Memory::LCD_STATUS );
    const auto currentMode = static_cast<PpuMode>( status & 0x3 );
    uint8_t ly = mem.read( Memory::LCD_Y );

    // State machine to handle PPU modes
    state.scanlineCycleNr++;

    switch( currentMode ) {
        using enum PpuMode;
    case H_BLANK:
        if( state.scanlineCycleNr >= scanlineDuration ) {
            state.scanlineCycleNr = 0;

            ly++;
            mem.write( Memory::LCD_Y, ly );

            if( ly >= 144 ) {
                status = status | static_cast<uint8_t>( V_BLANK );
                mem.write( Memory::LCD_STATUS, status );
                mem.setOamLock( false );

                // Request V-Blank interrupt
                mem.write( Memory::INTERRUPT_FLAG,
                           static_cast<uint8_t>( mem.read( Memory::INTERRUPT_FLAG ) | 0x01u ) );
            } else {
                status = status | static_cast<uint8_t>( OAM_SEARCH );
                mem.write( Memory::LCD_STATUS, status );
                mem.setOamLock( true );
            }
        }
        break;

    case V_BLANK:
        if( state.scanlineCycleNr >= scanlineDuration ) {
            state.scanlineCycleNr = 0;

            ly++;
            if( ly >= 154 ) {
                // End of V-Blank, back to first scanline
                ly = 0;
                mem.write( Memory::LCD_Y, ly );

                status = ( status & ~0x3 ) | static_cast<uint8_t>( OAM_SEARCH );
                mem.write( Memory::LCD_STATUS, status );
                mem.setOamLock( true );
            } else {
                mem.write( Memory::LCD_Y, ly );
            }
        }
        break;

    case OAM_SEARCH:
        if( state.scanlineCycleNr >= 80 ) { // OAM search lasts 80 cycles
            // At least for now do it in one go
            oamScan();

            // Reset FIFO state for next line
            state.currentX = 0;
            state.bgPixelsFifo.clear();
            state.spritePixelsFifo.clear();

            status = status | static_cast<uint8_t>( PIXEL_TRANSFER );
            mem.write( Memory::LCD_STATUS, status );
            mem.setVramLock( true );
        }
        break;

    case PIXEL_TRANSFER:
        if( state.bgPixelsFifo.size() == 0 && state.currentX < displayWidth ) {
            fetch();
            auto bg = state.bgPixelsFifo.popBatch();
            for( unsigned i = 0; i < 8 && state.currentX < displayWidth; i++ ) {
                drawPixel( mergePixel( bg[i], {} ) );
                state.currentX++;
            }
            state.bgPixelsFifo.clear();
            state.spritePixelsFifo.clear();
        }

        // Check if we're done rendering this line
        if( state.currentX >= displayWidth ) {
            // Move to H-Blank
            status = status & ~0x3;
            mem.write( Memory::LCD_STATUS, status );
            mem.setVramLock( false );
            mem.setOamLock( false );
        }
        break;
    }
    state.currentX++;
}

uint8_t PPU::mergePixel( Pixel bgPixel, Pixel spritePixel ) {
    // Merge background and object pixels
    const uint8_t lcdc = mem.read( Memory::LCD_CONTROL );
    const bool bgEnabled = lcdc & 0x01;
    const bool objEnabled = lcdc & 0x02;

    const uint8_t bgPalette = mem.read( Memory::BG_PALETTE );
    const uint8_t objPalette0 = mem.read( Memory::OBJECT_PALETTE_0 );
    const uint8_t objPalette1 = mem.read( Memory::OBJECT_PALETTE_1 );

    uint8_t finalColor;
    // Determine which pixel to display according to priority rules
    if( objEnabled && spritePixel.colorId != 0 ) {
        // Sprite pixel is not transparent
        if( bgEnabled && bgPixel.colorId != 0 && spritePixel.priority ) {
            // Background has priority over this sprite and is not transparent
            finalColor = bgPalette >> ( bgPixel.colorId * 2 ) & 0x03;
        } else {
            // Sprite has priority or background is transparent/disabled
            const uint8_t objPalette = spritePixel.palette ? objPalette1 : objPalette0;
            finalColor = objPalette >> ( spritePixel.colorId * 2 ) & 0x03;
        }
    } else if( bgEnabled ) {
        finalColor = bgPalette >> ( bgPixel.colorId * 2 ) & 0x03;
    } else {
        finalColor = 0;
    }
    return finalColor;
}
