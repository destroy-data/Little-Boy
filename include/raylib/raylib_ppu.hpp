#pragma once
#include "core/ppu.hpp"
#include <raylib.h>

class RaylibPpu final : public CorePpu {
private:
    Color* screenBuffer;
    void drawPixel( uint8_t colorId ) override;

public:
    Color* getScreenBuffer();
    RaylibPpu( IBus& bus_ );
    ~RaylibPpu() override;
};
