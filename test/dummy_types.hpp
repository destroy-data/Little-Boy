#include "core/cartridge.hpp"
#include "core/core_constants.hpp"
#include "core/cpu.hpp"
#include "core/emulator.hpp"
#include "core/ppu.hpp"
#include <cstdint>

class DummyCartridge final : public CoreCartridge {
    friend class Tester;

public:
    uint8_t read( uint16_t ) {
        return 0xFF;
    }
    void write( uint16_t, uint8_t ) {
    }
    DummyCartridge() : CoreCartridge( std::vector<uint8_t>( addr::globalChecksumEnd + 1 ) ) {
    }
};

class DummyCpu final : public Cpu {
    friend class Tester;

public:
    using Cpu::mopQueue;
    using Cpu::PC;
    using Cpu::registers;
};

class DummyPpu : public CorePpu {
    friend class Tester;

public:
    void drawPixel( uint8_t ) {
    }

    DummyPpu( IBus& bus_ ) : CorePpu( bus_ ) {
    }
};

class DummyBus : public IBus {
    friend class Tester;

public:
    uint8_t read( [[maybe_unused]] uint16_t address ) const override {
        return constant::invalidReadValue;
    }
    void write( [[maybe_unused]] uint16_t address, [[maybe_unused]] uint8_t value ) override {
    }
    void setOamLock( [[maybe_unused]] bool locked ) override {
    }
    void setVramLock( [[maybe_unused]] bool locked ) override {
    }
    SpriteAttribute getSpriteAttribute( [[maybe_unused]] uint8_t sprite_index ) const override {
        return { constant::invalidReadValue, constant::invalidReadValue, constant::invalidReadValue,
                 constant::invalidReadValue };
    }
    uint8_t directMemRead( [[maybe_unused]] uint16_t address ) const override {
        return constant::invalidReadValue;
    }
    void directMemWrite( [[maybe_unused]] uint16_t address, [[maybe_unused]] uint8_t value ) override {
    }
};

using DummyEmulator_t = Emulator<DummyPpu>;
