#include "core/bus.hpp"
#include <cstdint>

void setupBackgroundChessboardPatternInVram( IBus& bus );

void setupTestSprites( IBus& bus );

void createTestSprite( IBus& bus, int index, uint8_t x, uint8_t y, uint8_t tileId, uint8_t attributes );

void setupSpriteTiles( IBus& bus );

void setupLcdRegisters( IBus& bus );
