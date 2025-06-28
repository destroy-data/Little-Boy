#pragma once
#include "core/cpu.hpp"
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


//--------------------------------------------------
class RaylibCpu final : public CoreCpu {
public:
    void handleJoypad() override;
    RaylibCpu( IBus& bus_ ) : CoreCpu( bus_ ) {};
    ~RaylibCpu() override = default;
};
