#include "core/memory.hpp"
#include "dummy_types.hpp"
#include "ppu_helper.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

class TestPpu : public CorePpu {
    void drawPixel( uint8_t color ) {
        drawBuff.push_back( color );
    }

public:
    std::vector<uint8_t> drawBuff;
    TestPpu( Memory& mem_ ) : CorePpu( mem_ ) {
        std::memset( mem.oam, 0, 160 );
    }
};

TEST_CASE( "oam_scan", "[oam]" ) {
    Emulator<DummyCartridge, Memory, DummyCpu, TestPpu> emu;
    emu.memory.write( addr::lcdControl, 0x0 );
    createTestSprite( emu.memory.oam, 0, 1, 1, 0, 0 );

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

TEST_CASE( "headless_rendering", "[background][chessboard]" ) {
    Emulator<DummyCartridge, Memory, DummyCpu, TestPpu> emu;
    setupLcdRegisters( emu.memory );
    setupBackgroundChessboardPatternInVram( emu.memory.videoRam );
    setupTestSprites( emu.memory );

    for( int i = 0; i < CorePpu::displayHeight; i++ ) {
        for( int j = 0; j < CorePpu::scanlineDuration; j++ ) {
            emu.ppu.tick();
        }
    }
    REQUIRE( emu.ppu.drawBuff.size() == CorePpu::displayWidth * CorePpu::displayHeight );
}
