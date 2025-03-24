#include "core/cartridge.hpp"
#include "core/cpu.hpp"
#include "core/emulator.hpp"
#include "core/ppu.hpp"
#include <cstdint>

class DummyCartridge : public CoreCartridge {
    friend class Tester;
    using CoreCartridge::CoreCartridge;
    uint8_t read( uint16_t ) {
        return 0xFF;
    }
    void write( uint16_t, uint8_t ) {
    }
};

class DummyCpu : public CoreCpu {
    friend class Tester;
    using CoreCpu::CoreCpu;
    void handleJoypad() {
    }

public:
    DummyCpu( Memory& mem_ ) : CoreCpu( mem_ ) {};
};

class DummyPpu : public CorePpu {
    friend class Tester;
    void drawPixel( uint8_t ) {
    }

public:
    DummyPpu( Memory& mem_ ) : CorePpu( mem_ ) {
    }
};

using DummyEmulator = Emulator<DummyCartridge, Memory, DummyCpu, DummyPpu>;
