#include "core/bus.hpp"
#include "core/core_constants.hpp"
#include <cstdint>
#include <cstring>


void setupBackgroundChessboardPatternInVram( IBus& bus ) {
    for( unsigned i = 0; i < size::videoRam; i++ )
        bus.write( static_cast<uint16_t>( addr::videoRam + i ), 0 );
    // Tile 0: Solid white (all bytes 0x00, already set)
    // Tile 1: Solid black (all bits set to 1)
    for( int i = 16; i < 32; i++ ) {
        bus.write( static_cast<uint16_t>( addr::videoRam + i ), 0xFF );
    }

    // Set up background map as a chessboard pattern
    // The background map starts at 0x1800 in VRAM and is a 32x32 grid of tile numbers
    const auto chessboardSize = 4; // 4x4 tiles per square
    for( int y = 0; y < 32; y++ ) {
        for( int x = 0; x < 32; x++ ) {
            const uint16_t mapOffset = static_cast<uint16_t>( addr::tileMap1 + y * 32 + x );

            // Determine if this tile is in a black square
            // Using integer division to create the chessboard pattern
            bool isBlackSquare = ( ( x / chessboardSize ) + ( y / chessboardSize ) ) % 2;

            // Set the tile value (0 for white, 1 for black)
            bus.write( mapOffset, static_cast<uint8_t>( isBlackSquare ) );
        }
    }
}


void setupSpriteTiles( [[maybe_unused]] IBus& bus ) {
    /*    // Clear the first few tiles to ensure a clean slate
    for( int tile = 0; tile < 16; tile++ ) {
        const int tileOffset = tile * 16;
        for( int i = 0; i < 16; i++ ) {
            vram[tileOffset + i] = 0;
        }
    }

    // Tile 0: Empty tile (transparent)
    // Left as all zeros

    // Tile 1: Solid block
    const int solidTileOffset = 1 * 16;
    for( int i = 0; i < 16; i++ ) {
        vram[solidTileOffset + i] = 0xFF;
    }

    // Tile 2: Square outline (as in your original code)
    const int squareTileOffset = 2 * 16;
    // Top row (black)
    vram[squareTileOffset + 0] = 0xFF;
    vram[squareTileOffset + 1] = 0xFF;
    // Middle rows (black on sides, white in middle)
    for( int i = 2; i < 14; i += 2 ) {
        vram[squareTileOffset + i]     = 0xFF; // 10000001
        vram[squareTileOffset + i + 1] = 0x81; // 10000001
    }
    // Bottom row (black)
    vram[squareTileOffset + 14] = 0xFF;
    vram[squareTileOffset + 15] = 0xFF;

    // Tile 3: Cross (as in your original code)
    const int crossTileOffset = 3 * 16;
    // Set entire tile to 0 (white)
    for( int i = 0; i < 16; i++ ) {
        vram[crossTileOffset + i] = 0;
    }
    // Horizontal line
    vram[crossTileOffset + 6] = 0xFF;
    vram[crossTileOffset + 7] = 0xFF;
    // Vertical line
    for( int i = 0; i < 16; i += 2 ) {
        vram[crossTileOffset + i] |= 0x18;     // 00011000
        vram[crossTileOffset + i + 1] |= 0x18; // 00011000
    }

    // Tile 4: Circle
    const int circleTileOffset = 4 * 16;
    // Set entire tile to 0 (white)
    for( int i = 0; i < 16; i++ ) {
        vram[circleTileOffset + i] = 0;
    }
    // Top and bottom rows (partial circles)
    vram[circleTileOffset + 0]  = 0x3C; // 00111100
    vram[circleTileOffset + 1]  = 0x3C; // 00111100
    vram[circleTileOffset + 14] = 0x3C; // 00111100
    vram[circleTileOffset + 15] = 0x3C; // 00111100
    // Middle rows
    for( int i = 2; i < 14; i += 2 ) {
        if( i == 2 || i == 12 ) {
            vram[circleTileOffset + i]     = 0x7E; // 01111110
            vram[circleTileOffset + i + 1] = 0x7E; // 01111110
        } else {
            vram[circleTileOffset + i]     = 0x81; // 10000001
            vram[circleTileOffset + i + 1] = 0x81; // 10000001
        }
    }

    // Tile 5: Triangle (pointing up)
    const int triangleTileOffset = 5 * 16;
    // Set entire tile to 0 (white)
    for( int i = 0; i < 16; i++ ) {
        vram[triangleTileOffset + i] = 0;
    }
    // Bottom row (full black)
    vram[triangleTileOffset + 14] = 0xFF;
    vram[triangleTileOffset + 15] = 0xFF;
    // Building triangle from bottom to top
    vram[triangleTileOffset + 12] = 0x7E; // 01111110
    vram[triangleTileOffset + 13] = 0x7E; // 01111110
    vram[triangleTileOffset + 10] = 0x3C; // 00111100
    vram[triangleTileOffset + 11] = 0x3C; // 00111100
    vram[triangleTileOffset + 8]  = 0x18; // 00011000
    vram[triangleTileOffset + 9]  = 0x18; // 00011000
    vram[triangleTileOffset + 6]  = 0x08; // 00001000
    vram[triangleTileOffset + 7]  = 0x08; // 00001000

    // Tile 6: Diamond
    const int diamondTileOffset = 6 * 16;
    // Set entire tile to 0 (white)
    for( int i = 0; i < 16; i++ ) {
        vram[diamondTileOffset + i] = 0;
    }
    // Diamond pattern
    vram[diamondTileOffset + 0]  = 0x08; // 00001000
    vram[diamondTileOffset + 1]  = 0x08; // 00001000
    vram[diamondTileOffset + 2]  = 0x1C; // 00011100
    vram[diamondTileOffset + 3]  = 0x1C; // 00011100
    vram[diamondTileOffset + 4]  = 0x3E; // 00111110
    vram[diamondTileOffset + 5]  = 0x3E; // 00111110
    vram[diamondTileOffset + 6]  = 0x7F; // 01111111
    vram[diamondTileOffset + 7]  = 0x7F; // 01111111
    vram[diamondTileOffset + 8]  = 0x3E; // 00111110
    vram[diamondTileOffset + 9]  = 0x3E; // 00111110
    vram[diamondTileOffset + 10] = 0x1C; // 00011100
    vram[diamondTileOffset + 11] = 0x1C; // 00011100
    vram[diamondTileOffset + 12] = 0x08; // 00001000
    vram[diamondTileOffset + 13] = 0x08; // 00001000

    // Tile 7: Checkerboard pattern
    const int checkerTileOffset = 7 * 16;
    for( int i = 0; i < 16; i += 4 ) {
        vram[checkerTileOffset + i]     = 0xAA; // 10101010
        vram[checkerTileOffset + i + 1] = 0xAA; // 10101010
        vram[checkerTileOffset + i + 2] = 0x55; // 01010101
        vram[checkerTileOffset + i + 3] = 0x55; // 01010101
    }

    // Tile 8: Arrow (pointing right)
    const int arrowTileOffset = 8 * 16;
    // Set entire tile to 0 (white)
    for( int i = 0; i < 16; i++ ) {
        vram[arrowTileOffset + i] = 0;
    }
    // Arrow pattern
    vram[arrowTileOffset + 4]  = 0x10; // 00010000
    vram[arrowTileOffset + 5]  = 0x10; // 00010000
    vram[arrowTileOffset + 6]  = 0x18; // 00011000
    vram[arrowTileOffset + 7]  = 0x18; // 00011000
    vram[arrowTileOffset + 8]  = 0xFF; // 11111111
    vram[arrowTileOffset + 9]  = 0xFF; // 11111111
    vram[arrowTileOffset + 10] = 0x18; // 00011000
    vram[arrowTileOffset + 11] = 0x18; // 00011000
    vram[arrowTileOffset + 12] = 0x10; // 00010000
    vram[arrowTileOffset + 13] = 0x10; // 00010000
*/
}

void createTestSprite( IBus& bus, int index, uint8_t x, uint8_t y, uint8_t tileId, uint8_t attributes ) {
    const uint16_t addrBase = static_cast<uint16_t>( addr::objectAttributeMemory + index * 4 );

    // Y position (add 16 as Y=0 is 16 pixels above the screen)
    bus.write( addrBase, 16 + y );

    // X position (add 8 as X=0 is 8 pixels left of the screen)
    bus.write( addrBase + 1, 8 + x );

    // Tile ID
    bus.write( addrBase + 2, tileId );

    // Attributes (flags)
    bus.write( addrBase + 3, attributes );
}

void setupTestSprites( IBus& bus ) {
    // Set up sprite tiles in VRAM
    setupSpriteTiles( bus );

    // Create test sprites
    // Arguments: memory, OAM index, x, y, tile ID, attributes

    // Square sprite at the top left
    createTestSprite( bus, 0, 27, 27, 2, 0 );

    // Cross sprite at the top right
    createTestSprite( bus, 1, 100, 20, 3, 0 );

    // Square sprite with horizontal flip at bottom left
    createTestSprite( bus, 2, 20, 100, 2, ( 1 << 5 ) ); // Bit 5 = X-flip

    // Cross sprite with vertical flip at bottom right
    createTestSprite( bus, 3, 100, 100, 3, ( 1 << 6 ) ); // Bit 6 = Y-flip

    // Square sprite with low priority (behind background)
    createTestSprite( bus, 4, 60, 60, 2, ( 1 << 7 ) ); // Bit 7 = BG Priority

    // Cross sprite using alternate palette
    createTestSprite( bus, 5, 80, 40, 3, ( 1 << 4 ) ); // Bit 4 = Palette numberppu_hel
}

void setupLcdRegisters( IBus& bus ) {
    // Set LCD Control register (LCDC) - Enable display, BG, etc.
    bus.write( addr::lcdControl, 0x91 ); // 10010001
                                         // Bit 7: LCD enabled
                                         // Bit 0: BG & Window display enabled
                                         // Bit 4: BG & Window tile data at 0x8000-0x8FFF

    // Set scroll registers to 0
    bus.write( addr::bgScrollX, 0 );
    bus.write( addr::bgScrollY, 0 );

    // Set window position off-screen by default
    bus.write( addr::winX, 166 );
    bus.write( addr::winY, 144 );

    // Set palettes for DMG mode
    bus.write( addr::bgPalette, 0xE4 );      // 11100100 - Darkest to lightest
    bus.write( addr::objectPalette0, 0xE4 ); // 11100100 - Same as BG
    bus.write( addr::objectPalette1, 0xD2 ); // 11010010 - Alternate palette

    // Set LCD Y-coordinate to 0
    bus.write( addr::lcdY, 0 );

    // Set LYC to 0
    bus.write( addr::lyc, 0 );

    // Set LCD status register
    bus.write( addr::lcdStatus, 0x82 ); // 10000010 -> Mode set to OAM_SEARCH
}
