#include "core/memory.hpp"
#include <cstdint>

uint8_t Memory::read( const uint16_t index ) const {
    if( inRom00( index ) or inRom0N( index ) or inExternalRam( index ) )
        return cartridge->read( index );
    if( inVideoRam( index ) ) {
        if( vramLock )
            return 0xFF;
        return videoRam[index - addr::videoRam];
    }
    if( inWorkRam00( index ) )
        return workRam00[index - addr::workRam00];
    if( inWorkRam0N( index ) )
        return workRam0N[index - addr::workRam0N];
    if( inEchoRam00( index ) )
        return workRam00[index - addr::echoRam00];
    if( inEchoRam0N( index ) ) //echo RAM 0N is smaller than work RAM 0N
        return workRam0N[index - addr::echoRam0N];
    if( inObjectAttributeMemory( index ) ) {
        if( oamLock )
            return 0xFF;
        return oam[index - addr::objectAttributeMemory];
    }
    // TODO else if (index < NOT_USABLE + X)
    if( inIoRegisters( index ) )
        return ioRegisters[index - addr::ioRegisters];
    if( inHighRam( index ) )
        return highRam[index - addr::highRam];
    if( index == addr::interruptEnableRegister )
        return interruptEnableRegister;
    return 0;
}

void Memory::write( const uint16_t index, uint8_t value ) {
    if( inRom00( index ) or inRom0N( index ) or inExternalRam( index ) )
        cartridge->write( index, value );
    else if( inVideoRam( index ) )
        videoRam[index - addr::videoRam] = value;
    else if( inWorkRam00( index ) )
        workRam00[index - addr::workRam00] = value;
    else if( inWorkRam0N( index ) )
        workRam0N[index - addr::workRam0N] = value;
    else if( inEchoRam00( index ) )
        workRam00[index - addr::echoRam00] = value;
    else if( inEchoRam0N( index ) ) //echo RAM 0N is smaller than work RAM 0N
        workRam0N[index - addr::echoRam0N] = value;
    else if( inObjectAttributeMemory( index ) )
        oam[index - addr::objectAttributeMemory] = value;
    // TODO else if (index < NOT_USABLE + X)
    else if( inIoRegisters( index ) ) {
        ioRegisters[index - addr::ioRegisters] = value;
        // side effects
        if( index == addr::lcdY ) {
            if( value == read( addr::lyc ) ) {
                ioRegisters[addr::lcdStatus - addr::ioRegisters] |= ( 1 << 2 );
                //TODO interrupt
            } else
                ioRegisters[addr::lcdStatus - addr::ioRegisters] &= ~( 1 << 2 );
        }
    } else if( inHighRam( index ) )
        highRam[index - addr::highRam] = value;
    else if( index == addr::interruptEnableRegister )
        interruptEnableRegister = value;
}

void Memory::setVramLock( bool locked ) {
    vramLock = locked;
}

void Memory::setOamLock( bool locked ) {
    oamLock = locked;
}

Memory::Memory( CoreCartridge* cartridge_ ) : cartridge( cartridge_ ) {
    // DMG
    write( 0xFF00, 0xCF ); // P1
    write( 0xFF01, 0x00 ); // SB
    write( 0xFF02, 0x7E ); // SC
    write( 0xFF04, 0xAB ); // DIV
    write( 0xFF05, 0x00 ); // TIMA
    write( 0xFF06, 0x00 ); // TMA
    write( 0xFF07, 0xF8 ); // TAC
    write( 0xFF0F, 0xE1 ); // IF
    write( 0xFF10, 0x80 ); // NR10
    write( 0xFF11, 0xBF ); // NR11
    write( 0xFF12, 0xF3 ); // NR12
    write( 0xFF13, 0xFF ); // NR13
    write( 0xFF14, 0xBF ); // NR14
    write( 0xFF16, 0x3F ); // NR21
    write( 0xFF17, 0x00 ); // NR22
    write( 0xFF18, 0xFF ); // NR23
    write( 0xFF19, 0xBF ); // NR24
    write( 0xFF1A, 0x7F ); // NR30
    write( 0xFF1B, 0xFF ); // NR31
    write( 0xFF1C, 0x9F ); // NR32
    write( 0xFF1D, 0xFF ); // NR33
    write( 0xFF1E, 0xBF ); // NR34
    write( 0xFF20, 0xFF ); // NR41
    write( 0xFF21, 0x00 ); // NR42
    write( 0xFF22, 0x00 ); // NR43
    write( 0xFF23, 0xBF ); // NR44
    write( 0xFF24, 0x77 ); // NR50
    write( 0xFF25, 0xF3 ); // NR51
    write( 0xFF26, 0xF1 ); // NR52
    write( 0xFF40, 0x91 ); // LCDC
    write( 0xFF41, 0x85 ); // STAT
    write( 0xFF42, 0x00 ); // SCY
    write( 0xFF43, 0x00 ); // SCX
    write( 0xFF44, 0x00 ); // LY
    write( 0xFF45, 0x00 ); // LYC
    write( 0xFF46, 0xFF ); // DMA
    write( 0xFF47, 0xFC ); // BGP
    write( 0xFF48, 0xFF ); // OBP0 - uninitialized, usually either 0x0 or 0xFF
    write( 0xFF49, 0xFF ); // OBP1 - uninitialized, usually either 0x0 or 0xFF
    write( 0xFF4A, 0x00 ); // WY
    write( 0xFF4B, 0x00 ); // WX
    write( 0xFFFF, 0x00 ); // INTERRUPT ENABLE
}
