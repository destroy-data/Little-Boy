#include "core/memory.hpp"
#include <cstdint>

void setupBackgroundChessboardPatternInVram( uint8_t vram[] );

void setupTestSprites( Memory& mem );

void createTestSprite( uint8_t oam[], int index, uint8_t x, uint8_t y, uint8_t tileId,
                       uint8_t attributes );

void setupSpriteTiles( uint8_t vram[] );

void setupLcdRegisters( Memory& mem );
