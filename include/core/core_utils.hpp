#pragma once
#include <cstdint>

uint8_t lsb( std::same_as<uint16_t> auto value ) {
    return value & 0xFF;
}
uint8_t msb( std::same_as<uint16_t> auto value ) {
    return value & 0xFF00;
}
