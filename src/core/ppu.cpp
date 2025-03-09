#include <core/ppu.hpp>
#include <cstdint>

std::array<uint8_t, PPU::displayWidth * 3> PPU::draw() {
    const uint8_t lcdc = mem.read( LCD_CONTROL );
    const uint8_t ly = mem.read( LCD_Y );
    std::array<uint8_t, displayWidth * 3> lineBuffer;

    // Early return if LCD is disabled
    if( ~lcdc & ( 1 << 7 ) ) {
        // Fill with white when LCD is off
        for( unsigned i = 0; i < lineBuffer.size(); i++ )
            lineBuffer[i] = 255;

        return lineBuffer;
    }

    //mode 2 - search for objects which overlap current scanline
    std::span<uint8_t> object[10] = {};
    int objCount = 0;
    bool objSize8x8 = ~lcdc & ( 1 << 2 );

    // obj Y = pos + 16
    for( unsigned i = 0; i < sizeof( mem.oam ) && objCount < 10; i += 4 ) {
        if( ly >= mem.oam[i] - 16 && ly < mem.oam[i] - 8 * objSize8x8 ) {
            object[objCount++] = { &mem.oam[i], 4 }; //0-Y pos, 1-X pos
        }
    }

    // in non-CGB mode draw priority differs from selection priority, so sort the array
    // it's x position based, lower x is higher priority
    // if Xs are equal, first one in OAM has higher priority, fortunately insertion sort is stable
    for( int i = 1; i < objCount; i++ ) {
        const auto key = object[i];
        int j = i - 1;
        while( j >= 0 && object[j][1] > key[1] ) {
            object[j + 1] = object[j];
            j--;
        }
        object[j + 1] = key;
    }

    //For now draw whole line at once
    //TODO: don't
    uint8_t scrollX = mem.read( BG_SCROLL_X );
    uint8_t scrollY = mem.read( BG_SCROLL_Y );
    uint8_t winX = mem.read( WIN_X );
    uint8_t winY = mem.read( WIN_Y );
    bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
    bool useSecondBgMap = lcdc & ( 1 << 3 );
    bool useSecondWinMap = lcdc & ( 1 << 6 );
    bool bgWinEnabled = lcdc & 0x1;
    bool winEnabled = ( lcdc & ( 1 << 5 ) ) && ( winY <= ly ) && ( winX < 168 );
    bool objEnabled = lcdc & ( 1 << 1 );

    uint8_t palette = mem.read( BG_PALETTE );
    // extract color values (0-3) from each 2-bit position
    uint8_t bgColors[4] = { static_cast<uint8_t>( ( palette >> 0 ) & 0x3 ),
                            static_cast<uint8_t>( ( palette >> 2 ) & 0x3 ),
                            static_cast<uint8_t>( ( palette >> 4 ) & 0x3 ),
                            static_cast<uint8_t>( ( palette >> 6 ) & 0x3 ) };

    palette = mem.read( OBJECT_PALETTE_0 );
    uint8_t obp0Colors[4] = { 0, // Color 0 is always transparent for objects
                              static_cast<uint8_t>( ( palette >> 2 ) & 0x3 ),
                              static_cast<uint8_t>( ( palette >> 4 ) & 0x3 ),
                              static_cast<uint8_t>( ( palette >> 6 ) & 0x3 ) };

    palette = mem.read( OBJECT_PALETTE_1 );
    uint8_t obp1Colors[4] = { 0, // Color 0 is always transparent for objects
                              static_cast<uint8_t>( ( palette >> 2 ) & 0x3 ),
                              static_cast<uint8_t>( ( palette >> 4 ) & 0x3 ),
                              static_cast<uint8_t>( ( palette >> 6 ) & 0x3 ) };

    if( bgWinEnabled ) {
        for( uint8_t x = 0; x < displayWidth; x++ ) {
            bool windowPixel = winEnabled && ( x >= winX - 7 );
            uint8_t colorId = 0;

            if( windowPixel ) {
                auto pixX = static_cast<uint8_t>( x - winX + 7 );
                auto pixY = static_cast<uint8_t>( ly - winY );
                uint8_t winTileId = tilemap[useSecondWinMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress =
                        base8000Addr ? tileSize * winTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( winTileId );
                auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
            } else {
                auto pixX = static_cast<uint8_t>( x + scrollX );
                auto pixY = static_cast<uint8_t>( ly + scrollY );
                auto bgTileId = tilemap[useSecondBgMap][pixY / 8 * 32 + pixX / 8];
                const int tileAddress =
                        base8000Addr ? tileSize * bgTileId
                                     : 0x1000 + tileSize * static_cast<int8_t>( bgTileId );
                auto tile = Tile_t( &mem.videoRam[tileAddress], Tile_t::extent );
                colorId = getPixelColor( tile, pixX % 8, pixY % 8 );
            }

            for( int i = 0; i < 3; i++ )
                lineBuffer[3 * x + i] = colorMap[colorId][i];
        }
    } else {
        for( unsigned i = 0; i < lineBuffer.size(); i++ )
            lineBuffer[i] = 255;
    }
    //TODO: only check if it can overpaint bg/win (CGB)
    if( objEnabled ) {
        for( int i = 0; i < objCount; i++ ) {
            uint8_t objX = object[i][1] - 8;
            for( int x = objX; x < objX + 8; x++ ) {
                uint8_t objY = object[i][0] - 16;
                //check if pixel is transparent
                uint8_t tileId = object[i][2];
                if( !objSize8x8 )
                    tileId &= 0xFE;
                const auto tile = std::span( ( base8000Addr ? mem.videoRam : mem.videoRam ) +
                                                     tileSize * tileId,
                                             objSize8x8 ? 16 : 32 );
                uint8_t attributes = object[i][3];
                bool xFlip = attributes & ( 1 << 5 );
                bool yFlip = attributes & ( 1 << 6 );
                bool priority = attributes & ( 1 << 7 );
                bool dmgUseOBP0 = attributes & ( 1 << 4 ); // DMG only
                uint8_t cgbPalette = attributes & 0x7;     // CGB only

                uint8_t inSpriteX = x - objX;
                if( xFlip )
                    inSpriteX = 7 - inSpriteX;
                uint8_t inSpriteY = ly - objY;
                if( yFlip )
                    inSpriteY = static_cast<uint8_t>( ( objSize8x8 ? 7 : 15 ) - inSpriteY );

                auto colorId = static_cast<uint8_t>(
                        ( tile[inSpriteY * 2] >> inSpriteX & 0x1 ) |
                        ( ( tile[inSpriteY * 2 + 1] >> inSpriteX & 0x1 ) << 1 ) );

                if( colorId ) {
                    //TODO
                    break;
                }
            }
        }
    }
}

uint8_t PPU::getPixelColor( Tile_t tile, int x, int y ) {
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
