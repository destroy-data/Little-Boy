#include "core/memory.hpp"
#include "dummy_types.hpp"
#include "ppu_helper.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class TestPpu : public CorePpu {
    void drawPixel( uint8_t color ) {
        drawBuff.push_back( color );
    }

public:
    std::vector<uint8_t> drawBuff;
    TestPpu( IBus& bus_ ) : CorePpu( bus_ ) {
        for( unsigned i = 0; i < size::oam; i++ )
            bus.write( static_cast<uint16_t>( size::oam + i ), 0 );
    }
};

void handleJoypad( [[maybe_unused]] IBus& bus ) {
}

TEST_CASE( "OAM scan", "[oam]" ) {
    // FIXME sometimes passes, sometimes not
    Emulator<TestPpu> emu( std::make_unique<DummyCartridge>(), handleJoypad );
    emu.memory.write( addr::lcdControl, 0x0 );
    createTestSprite( emu, 0, 1, 1, 0, 0 );

    emu.memory.write( addr::lcdY, 0 );
    emu.ppu.oamScan();
    REQUIRE( emu.ppu.state.objCount == 0 );

    emu.memory.write( addr::lcdY, 1 );
    emu.ppu.oamScan();
    REQUIRE( emu.ppu.state.objCount == 1 );

    emu.memory.write( addr::lcdY, 9 );
    emu.ppu.oamScan();
    REQUIRE( emu.ppu.state.objCount == 0 );

    //Turn on 8x16 sprites
    emu.memory.write( addr::lcdControl, 0x4 );
    emu.ppu.oamScan();
    REQUIRE( emu.ppu.state.objCount == 1 );
}

TEST_CASE( "Headless rendering", "[background][chessboard]" ) {
    Emulator<TestPpu> emu( std::make_unique<DummyCartridge>(), handleJoypad );
    setupLcdRegisters( emu );
    setupBackgroundChessboardPatternInVram( emu );
    setupTestSprites( emu );

    for( int i = 0; i < CorePpu::displayHeight; i++ ) {
        for( int j = 0; j < CorePpu::scanlineDuration; j++ ) {
            emu.ppu.tick();
        }
    }
    REQUIRE( emu.ppu.drawBuff.size() == CorePpu::displayWidth * CorePpu::displayHeight );
}
