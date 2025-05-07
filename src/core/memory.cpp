#include "core/memory.hpp"
#include <cstdint>

uint8_t Memory::read( const uint16_t index ) const {
    if( inRom00( index ) or inRom0N( index ) or inExternalRam( index ) )
        return cartridge.read( index );
    if( inVideoRam( index ) ) {
        if( vramLock )
            return 0xFF;
        return videoRam[index - VIDEO_RAM];
    }
    if( inWorkRam00( index ) )
        return workRam00[index - WORK_RAM_00];
    if( inWorkRam0N( index ) )
        return workRam0N[index - WORK_RAM_0N];
    if( inEchoRam00( index ) )
        return workRam00[index - ECHO_RAM_00];
    if( inEchoRam0N( index ) ) //echo RAM 0N is smaller than work RAM 0N
        return workRam0N[index - ECHO_RAM_0N];
    if( inObjectAttributeMemory( index ) ) {
        if( oamLock )
            return 0xFF;
        return oam[index - OBJECT_ATTRIBUTE_MEMORY];
    }
    // TODO else if (index < NOT_USABLE + X)
    if( inIoRegisters( index ) )
        return ioRegisters[index - IO_REGISTERS];
    if( inHighRam( index ) )
        return highRam[index - HIGH_RAM];
    if( index == INTERRUPT_ENABLE_REGISTER )
        return interruptEnableRegister;
    return 0;
}
void Memory::write( const uint16_t index, uint8_t value ) {
    if( inRom00( index ) or inRom0N( index ) or inExternalRam( index ) )
        cartridge.write( index, value );
    else if( inVideoRam( index ) )
        videoRam[index - VIDEO_RAM] = value;
    else if( inWorkRam00( index ) )
        workRam00[index - WORK_RAM_00] = value;
    else if( inWorkRam0N( index ) )
        workRam0N[index - WORK_RAM_0N] = value;
    else if( index < ECHO_RAM_00 + sizeof( workRam00 ) )
        workRam00[index - ECHO_RAM_00] = value;
    else if( inEchoRam0N( index ) ) //echo RAM 0N is smaller than work RAM 0N
        workRam0N[index - ECHO_RAM_0N] = value;
    else if( inObjectAttributeMemory( index ) )
        oam[index - OBJECT_ATTRIBUTE_MEMORY] = value;
    // TODO else if (index < NOT_USABLE + X)
    else if( inIoRegisters( index ) ) {
        ioRegisters[index - IO_REGISTERS] = value;
        // side effects
        if( index == LCD_Y ) {
            if( value == read( LYC ) ) {
                ioRegisters[(uint16_t)LCD_STATUS - IO_REGISTERS] |= ( 1 << 2 );
                //TODO interrupt
            } else
                ioRegisters[(uint16_t)LCD_STATUS - IO_REGISTERS] &= ~( 1 << 2 );
        }
    } else if( inHighRam( index ) )
        highRam[index - HIGH_RAM] = value;
    else if( index == INTERRUPT_ENABLE_REGISTER )
        interruptEnableRegister = value;
}
void Memory::setVramLock( bool locked ) {
    vramLock = locked;
}
void Memory::setOamLock( bool locked ) {
    oamLock = locked;
}
