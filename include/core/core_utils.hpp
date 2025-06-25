#pragma once
#include <concepts>
#include <cstdint>

constexpr uint8_t lsb( std::integral auto value ) {
    return static_cast<uint8_t>( value & 0xFF );
}
// msb deliberately accepts only uint16_t, because it's easy to trigger integral promotion,
// which would have unexpected result on returned value
constexpr uint8_t msb( std::same_as<uint16_t> auto value ) {
    return static_cast<uint8_t>( ( value >> 8 ) & 0xFF );
}
