#include "core/memory.hpp"
#include <cstdint>

uint8_t Memory::read( const uint16_t index ) const {
    if( index < VIDEO_RAM || ( index >= EXTERNAL_RAM && index <= WORK_RAM_00 ) )
        return cartridge.read( index );
    else if( index < VIDEO_RAM + sizeof( videoRam ) ) {
        if( vramLock )
            return 0xFF;
        else
            return videoRam[index - VIDEO_RAM];
    } else if( index < WORK_RAM_00 + sizeof( workRam00 ) )
        return workRam00[index - WORK_RAM_00];
    else if( index < WORK_RAM_0N + sizeof( workRam0N ) )
        return workRam0N[index - WORK_RAM_0N];
    else if( index < ECHO_RAM_00 + sizeof( workRam00 ) )
        return workRam00[index - ECHO_RAM_00];
    else if( index < OBJECT_ATTRIBUTE_MEMORY ) //echo RAM 0N is smaller than work RAM 0N
        return workRam0N[index - ECHO_RAM_0N];
    else if( index < OBJECT_ATTRIBUTE_MEMORY + sizeof( oam ) ) {
        if( oamLock )
            return 0xFF;
        else
            return oam[index - OBJECT_ATTRIBUTE_MEMORY];
    }
    // TODO else if (index < NOT_USABLE + X)
    else if( index < IO_REGISTERS + sizeof( ioRegisters ) )
        return ioRegisters[index - IO_REGISTERS];
    else if( index < HIGH_RAM + sizeof( highRam ) )
        return highRam[index - HIGH_RAM];
    else if( index == 0xFFFF )
        return interruptEnableRegister;
}

void Memory::write( const uint16_t index, uint8_t value ) {
    if( index < VIDEO_RAM || ( index >= EXTERNAL_RAM && index <= WORK_RAM_00 ) )
        cartridge.write( index, value );
    else if( index < VIDEO_RAM + sizeof( videoRam ) )
        videoRam[index - VIDEO_RAM] = value;
    else if( index < WORK_RAM_00 + sizeof( workRam00 ) )
        workRam00[index - WORK_RAM_00] = value;
    else if( index < WORK_RAM_0N + sizeof( workRam0N ) )
        workRam0N[index - WORK_RAM_0N] = value;
    else if( index < ECHO_RAM_00 + sizeof( workRam00 ) )
        workRam00[index - ECHO_RAM_00] = value;
    else if( index < OBJECT_ATTRIBUTE_MEMORY ) //echo RAM 0N is smaller than work RAM 0N
        workRam0N[index - ECHO_RAM_0N] = value;
    else if( index < OBJECT_ATTRIBUTE_MEMORY + sizeof( oam ) )
        oam[index - OBJECT_ATTRIBUTE_MEMORY] = value;
    // TODO else if (index < NOT_USABLE + X)
    else if( index < IO_REGISTERS + sizeof( ioRegisters ) )
        ioRegisters[index - IO_REGISTERS] = value;
    else if( index < HIGH_RAM + sizeof( highRam ) )
        highRam[index - HIGH_RAM] = value;
    else if( index == 0xFFFF )
        interruptEnableRegister = value;
}

void Memory::setVramLock( bool locked ) {
    vramLock = locked;
}

void Memory::setOamLock( bool locked ) {
    oamLock = locked;
}
