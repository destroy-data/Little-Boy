#pragma once
#include "core/bus.hpp"
#include <cstdint>

// Basic implementation - TODO edge cases
class Timer {
    enum class ClockSelect { every1024Tcycles, every16Tcycles, every64Tcycles, every256Tcycles };
    IBus& bus;
    // increment by one for every T-cycle; 1s is 2^22 T-cycles
    uint32_t masterCounter = 0;
    bool previousAndResult = false;

public:
    void tick();
    void write( uint16_t address, uint8_t value );
    Timer( IBus& bus_ ) : bus( bus_ ) {
    }
};
