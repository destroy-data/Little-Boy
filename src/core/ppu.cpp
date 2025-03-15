#include "core/memory.hpp"
#include <core/ppu.hpp>
#include <cstdint>

void PPU::oamScan() {
    //mode 2 - search for objects which overlap current scanline
    //it takes 80 dots
    state.objCount = 0;
    const bool objSize8x8 = ~mem.read( LCD_CONTROL ) & ( 1 << 2 );
    const int ly = mem.read( LCD_Y );

    // obj Y = pos + 16
    for( unsigned i = 0; i < sizeof( mem.oam ) && state.objCount < 10; i += 4 ) {
        if( ly >= mem.oam[i] - 16 && ly < mem.oam[i] - 8 * objSize8x8 ) {
            state.object[state.objCount++] = { &mem.oam[i], 4 }; //0-Y pos, 1-X pos
        }
    }

    // in non-CGB mode draw priority differs from selection priority, so sort the array
    // it's x position based, lower x is higher priority
    // if Xs are equal, first one in OAM has higher priority, fortunately insertion sort is stable
    for( int i = 1; i < state.objCount; i++ ) {
        const auto key = state.object[i];
        int j = i - 1;
        while( j >= 0 && state.object[j][1] > key[1] ) {
            state.object[j + 1] = state.object[j];
            j--;
        }
        state.object[j + 1] = key;
    }
}

std::array<PPU::Pixel, 8> PPU::fetch() {
    const uint8_t lcdc = mem.read( LCD_CONTROL );
    const bool bgWinEnabled = lcdc & 0x1;
    const uint8_t ly = mem.read( LCD_Y );
    const uint8_t winX = mem.read( WIN_X );
    const uint8_t winY = mem.read( WIN_Y );
    const uint8_t scrollX = mem.read( BG_SCROLL_X );
    const uint8_t scrollY = mem.read( BG_SCROLL_Y );
    const bool winEnabled = ( lcdc & ( 1 << 5 ) ) && ( winY <= ly ) && ( winX < 168 );
    const bool objEnabled = lcdc & ( 1 << 1 );
    const bool useSecondBgMap = lcdc & ( 1 << 3 );
    const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
    const bool useSecondWinMap = lcdc & ( 1 << 6 );

    std::array<Pixel, 8> bgBuffer {};
    std::array<Pixel, 8> objectBuffer {};
    const int end = state.currentX + 8;
    for( int i = 0; state.currentX < end; ++i, ++state.currentX ) {
        if( bgWinEnabled ) {
            const bool windowPixel = winEnabled && ( state.currentX >= winX - 7 );
            uint8_t colorId = 0;
            if( windowPixel ) {
                const auto pixX = static_cast<uint8_t>( state.currentX - winX + 7 );
                const auto pixY = static_cast<uint8_t>( ly - winY );
                const uint8_t winTileId = tilemap[useSecondWinMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress =
                        base8000Addr ? tileSize * winTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( winTileId );
                const auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
                bgBuffer[i] = { .color = colorId };
            } else {
                const auto pixX = static_cast<uint8_t>( state.currentX + scrollX );
                const auto pixY = static_cast<uint8_t>( ly + scrollY );
                const auto bgTileId = tilemap[useSecondBgMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress =
                        base8000Addr ? tileSize * bgTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( bgTileId );
                const auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
                bgBuffer[i] = { .color = colorId };
            }
        }
        if( objEnabled ) { //FIXME
            const int objHeight = ( ~lcdc & ( 1 << 2 ) ) ? 8 : 16;

            // For each object found during OAM scan
            for( int j = 0; j < state.objCount; j++ ) {
                const uint8_t objX = state.object[j][1] - 8;
                const uint8_t objY = state.object[j][0] - 16;

                if( objY + objHeight < ly || objY > ly )
                    continue;
                if( objX + 8 <= state.currentX || objX >= end )
                    continue;

                uint8_t tileId = state.object[j][2];
                if( objHeight == 16 )
                    tileId &= 0xFE;

                const uint8_t attributes = state.object[j][3];
                const bool xFlip = attributes & ( 1 << 5 );
                const bool yFlip = attributes & ( 1 << 6 );
                const bool priority = attributes & ( 1 << 7 );
                const bool dmgUseOBP1 = attributes & ( 1 << 4 );

                uint8_t tileY = ly - objY;
                if( yFlip )
                    tileY = objHeight - 1 - tileY;

                // objects always use 8000 addressing mode
                const auto tile =
                        Tile_t( &mem.videoRam[tileId * ( tileY < 8 ? tileSize : tileSize + 1 )],
                                Tile_t::extent );

                for( int x = state.currentX; x < end && x < objX + 8; x++ ) {
                    // Skip if pixel is outside object bounds
                    if( x < objX || x >= objX + 8 )
                        continue;

                    int fifoIndex = x - state.currentX;
                    uint8_t tileX = x - objX;
                    if( xFlip ) {
                        tileX = 7 - tileX;
                    }
                    uint8_t colorId = getPixelColor( tile, tileX, tileY );

                    // Color 0 is transparent for objects
                    if( objectBuffer[fifoIndex].color == 0 ) {
                        objectBuffer[fifoIndex] = {
                                .color = colorId, .palette = dmgUseOBP1, .priority = priority };
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
    const uint8_t lcdc = mem.read( LCD_CONTROL );
    if( !( lcdc & ( 1 << 7 ) ) ) {
        return; // LCD disabled, nothing to do
    }

    uint8_t stat = mem.read( LCD_STATUS );
    uint8_t currentMode = stat & 0x3;
    uint8_t ly = mem.read( LCD_Y );

    // State machine to handle PPU modes
    static int scanlineCycleNr = 0; // one scanline takes 456 cycles
    scanlineCycleNr++;

    switch( currentMode ) {
    case 0: // H-Blank
        if( scanlineCycleNr >= scanlineDuration ) {
            scanlineCycleNr = 0;

            ly++;
            mem.write( LCD_Y, ly );

            if( ly >= 144 ) {
                // Enter V-Blank mode
                stat = stat | 0x1;
                mem.write( LCD_STATUS, stat );

                // Request V-Blank interrupt
                mem.write( Memory::INTERRUPT_FLAG, mem.read( Memory::INTERRUPT_FLAG ) | 0x01 );
            } else {
                // Move to OAM search mode for next line
                stat = stat | 0x2;
                mem.write( LCD_STATUS, stat );
            }
        }
        break;

    case 1: // V-Blank
        if( scanlineCycleNr >= scanlineDuration ) {
            scanlineCycleNr = 0;

            ly++;
            if( ly >= 154 ) {
                // End of V-Blank, back to first scanline
                ly = 0;
                mem.write( LCD_Y, ly );

                // Move to OAM search
                stat = ( stat & ~0x3 ) | 0x2;
                mem.write( LCD_STATUS, stat );
            } else {
                mem.write( LCD_Y, ly );
            }
        }
        break;

    case 2:                           // OAM Search
        if( scanlineCycleNr >= 80 ) { // OAM search lasts 80 cycles
            scanlineCycleNr = 0;

            // At least for now do it in one go
            oamScan();

            // Reset FIFO state for next line
            state.currentX = 0;
            state.bgPixelsFifo.clear();
            state.spritePixelsFifo.clear();

            // Move to Pixel Transfer mode
            stat = stat | 0x3;
            mem.write( LCD_STATUS, stat );
        }
        break;

    case 3: // Pixel Transfer
        if( state.bgPixelsFifo.size() == 0 && state.currentX < displayWidth ) {
            fetch();
        }

        // Check if we're done rendering this line
        if( state.currentX >= displayWidth ) {
            scanlineCycleNr = 0;

            // Move to H-Blank
            stat = stat & ~0x3;
            mem.write( LCD_STATUS, stat );
        }
        break;
    }
}
