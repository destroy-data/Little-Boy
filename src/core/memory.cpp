#include "core/memory.hpp"

uint8_t& Memory::operator[]( const std::size_t index ) {
    //TODO
    //if(index>)
}

const uint8_t& Memory::operator()( const std::size_t index ) const {
    if( index < ROM_00 + sizeof( rom00 ) )
        return rom00[index];
    //ROM_01_TO_0N
    else if( index < VIDEO_RAM + sizeof( videoRam ) )
        return videoRam[index - VIDEO_RAM];
    else if( index < WORK_RAM_00 + sizeof( workRam00 ) )
        return workRam00[index - WORK_RAM_00];
    else if( index < WORK_RAM_0N + sizeof( workRam0N ) )
        return workRam0N[index - WORK_RAM_0N];
    else if( index < ECHO_RAM_00 + sizeof( workRam00 ) )
        return workRam00[index - ECHO_RAM_00];
    else if( index < OBJECT_ATTRIBUTE_MEMORY ) //echo RAM 0N is smaller than work RAM 0N
        return workRam0N[index - ECHO_RAM_0N];
    //TODO rest
}
