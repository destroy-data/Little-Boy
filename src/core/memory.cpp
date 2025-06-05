#include "core/memory.hpp"
#include "core/cartridge.hpp"
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
