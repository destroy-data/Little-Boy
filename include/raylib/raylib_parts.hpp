#pragma once
#include "core/cpu.hpp"
#include "core/ppu.hpp"
#include <raylib.h>

class RaylibPpu final : public CorePpu {
private:
    Color* screenBuffer;

protected:
    void drawPixel( uint8_t colorId ) override;

public:
    Color* getScreenBuffer();
    RaylibPpu( Memory& mem_ );
    ~RaylibPpu() override;
};


//--------------------------------------------------
class RaylibCpu final : public CoreCpu {
public:
    void handleJoypad() override;
    RaylibCpu( Memory& mem_ ) : CoreCpu( mem_ ) {};
    ~RaylibCpu() override = default;
};
