#pragma once
#include <concepts>
#include <cstdint>

uint8_t lsb( std::same_as<uint16_t> auto value ) {
    return static_cast<uint8_t>( value & 0xFF );
}
uint8_t msb( std::same_as<uint16_t> auto value ) {
    return static_cast<uint8_t>( ( value >> 8 ) & 0xFF );
}
