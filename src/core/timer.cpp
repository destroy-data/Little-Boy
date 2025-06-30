#include "core/timer.hpp"
#include "core/core_constants.hpp"

void Timer::tick() {
    masterCounter++;
    bus.write( addr::divider, static_cast<uint8_t>( masterCounter >> 8 ) );
    const auto timerControl = bus.read( addr::timerControl );
    const bool timaEnabled  = timerControl & ( 1 << 2 );
    unsigned mask           = 0;
    const auto clockSelect  = static_cast<ClockSelect>( timerControl & 0x3 );
    if( timaEnabled ) {
        switch( clockSelect ) {
            using enum ClockSelect;
        case every16Tcycles:
            mask = 1 << 3;
            break;
        case every64Tcycles:
            mask = 1 << 5;
            break;
        case every256Tcycles:
            mask = 1 << 7;
            break;
        case every1024Tcycles:
            mask = 1 << 9;
            break;
        }
    }

    bool newAndResult = masterCounter & mask;
    // increment on falling edge
    if( previousAndResult && ! newAndResult ) {
        uint8_t tima = bus.read( addr::timerCounter );

        if( tima == 0xFF ) {
            bus.write( addr::timerCounter, bus.read( addr::timerModulo ) );
            bus.write( addr::interruptFlag, bus.read( addr::interruptFlag ) | bitMask::timerInterrupt );
        } else {
            bus.write( addr::timerCounter, tima + 1 );
        }
    }
    previousAndResult = newAndResult;
}

void Timer::write( uint16_t address, uint8_t value ) {
    // TODO edgecases
    switch( address ) {
    case addr::divider:
        masterCounter = 0;
        bus.directMemWrite( address, 0 );
        break;
    case addr::timerCounter:
        bus.directMemWrite( address, value );
        break;
    case addr::timerModulo:
        bus.directMemWrite( address, value );
        break;
    case addr::timerControl:
        bus.directMemWrite( address, value );
        break;
    }
    return;
}
