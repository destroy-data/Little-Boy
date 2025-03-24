#include "core/memory.hpp"
#include <cstdint>
#include <cstring>


void setupBackgroundChessboardPatternInVram( uint8_t vram[] ) {
    std::memset( vram, 0, 8192 );

    // Tile 0: Solid white (all bytes 0x00, already set by memset)
    // Tile 1: Solid black (all bits set to 1)
    for( int i = 16; i < 32; i++ ) {
        vram[i] = 0xFF;
    }

    // Set up background map as a chessboard pattern
    // The background map starts at 0x1800 in VRAM and is a 32x32 grid of tile numbers
    for( int y = 0; y < 32; y++ ) {
        for( int x = 0; x < 32; x++ ) {
            unsigned mapOffset = 0x1800 + y * 32 + x;

            // Integer division by 2 creates 2x2 tile squares
            bool isBlackSquare = ( ( x / 2 ) + ( y / 2 ) ) % 2;

            // Set the tile value (0 for white, 1 for black)
            vram[mapOffset] = static_cast<uint8_t>( isBlackSquare );
        }
    }
}


void setupSpriteTiles( uint8_t vram[] ) {
    // Tile 2: Simple sprite pattern (square)
    // Each tile is 16 bytes (8x8 pixels, 2 bits per pixel)
    const int tileOffset = 2 * 16; // Tile ID 2

    // Top row (black)
    vram[tileOffset] = 0xFF;
    vram[tileOffset + 1] = 0xFF;

    // Middle rows (black on sides, white in middle)
    for( int i = 2; i < 14; i += 2 ) {
        vram[tileOffset + i] = 0x81;     // 10000001
        vram[tileOffset + i + 1] = 0x81; // 10000001
    }

    // Bottom row (black)
    vram[tileOffset + 14] = 0xFF;
    vram[tileOffset + 15] = 0xFF;

    // Tile 3: Simple sprite pattern (cross)
    const int crossTileOffset = 3 * 16; // Tile ID 3

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
}

void createTestSprite( uint8_t oam[], int index, uint8_t x, uint8_t y, uint8_t tileId,
                       uint8_t attributes ) {
    const int oamBase = index * 4;

    // Y position (add 16 as Y=0 is 16 pixels above the screen)
    oam[oamBase] = 16 + y;

    // X position (add 8 as X=0 is 8 pixels left of the screen)
    oam[oamBase + 1] = 8 + x;

    // Tile ID
    oam[oamBase + 2] = tileId;

    // Attributes (flags)
    oam[oamBase + 3] = attributes;
}

void setupTestSprites( Memory& mem ) {
    // Set up sprite tiles in VRAM
    setupSpriteTiles( mem.videoRam );

    // Create test sprites
    // Arguments: memory, OAM index, x, y, tile ID, attributes

    // Square sprite at the top left
    createTestSprite( mem.oam, 0, 20, 20, 2, 0 );

    // Cross sprite at the top right
    createTestSprite( mem.oam, 1, 100, 20, 3, 0 );

    // Square sprite with horizontal flip at bottom left
    createTestSprite( mem.oam, 2, 20, 100, 2, ( 1 << 5 ) ); // Bit 5 = X-flip

    // Cross sprite with vertical flip at bottom right
    createTestSprite( mem.oam, 3, 100, 100, 3, ( 1 << 6 ) ); // Bit 6 = Y-flip

    // Square sprite with low priority (behind background)
    createTestSprite( mem.oam, 4, 60, 60, 2, ( 1 << 7 ) ); // Bit 7 = BG Priority

    // Cross sprite using alternate palette
    createTestSprite( mem.oam, 5, 80, 40, 3, ( 1 << 4 ) ); // Bit 4 = Palette number
}

void setupLcdRegisters( Memory& mem ) {
    // Set LCD Control register (LCDC) - Enable display, BG, etc.
    mem.write( Memory::LCD_CONTROL, 0x91 ); // 10010001
                                            // Bit 7: LCD enabled
                                            // Bit 0: BG & Window display enabled
                                            // Bit 4: BG & Window tile data at 0x8000-0x8FFF

    // Set scroll registers to 0
    mem.write( Memory::BG_SCROLL_X, 0 );
    mem.write( Memory::BG_SCROLL_Y, 0 );

    // Set window position off-screen by default
    mem.write( Memory::WIN_X, 166 );
    mem.write( Memory::WIN_Y, 144 );

    // Set palettes for DMG mode
    mem.write( Memory::BG_PALETTE, 0xE4 );       // 11100100 - Darkest to lightest
    mem.write( Memory::OBJECT_PALETTE_0, 0xE4 ); // 11100100 - Same as BG
    mem.write( Memory::OBJECT_PALETTE_1, 0xD2 ); // 11010010 - Alternate palette

    // Set LCD Y-coordinate to 0
    mem.write( Memory::LCD_Y, 0 );

    // Set LYC to 0
    mem.write( Memory::LYC, 0 );

    // Set LCD status register
    mem.write( Memory::LCD_STATUS, 0x86 ); // 10000101
                                           // Bit 7: Always 1
                                           // Bit 2: LYC=LY coincidence flag
                                           // Bit 0-1: Mode flag (01 = V-Blank)
}
