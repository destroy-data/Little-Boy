#include "core/cartridge.hpp"
#include "core/cpu.hpp"
#include "core/emulator.hpp"
#include "core/ppu.hpp"
#include <cstdint>

class DummyCartridge final : public CoreCartridge {
    friend class Tester;
    using CoreCartridge::CoreCartridge;

public:
    uint8_t read( uint16_t ) {
        return 0xFF;
    }
    void write( uint16_t, uint8_t ) {
    }
    DummyCartridge() : CoreCartridge( std::vector<uint8_t> {} ) {
    }
};

class DummyCpu final : public CoreCpu {
    friend class Tester;

public:
    void handleJoypad() override {
    }
    DummyCpu( IBus& bus_ ) : CoreCpu( bus_ ) {};
};

class DummyPpu : public CorePpu {
    friend class Tester;

public:
    void drawPixel( uint8_t ) {
    }

    DummyPpu( IBus& bus_ ) : CorePpu( bus_ ) {
    }
};

using DummyEmulator_t = Emulator<DummyCpu, DummyPpu>;
