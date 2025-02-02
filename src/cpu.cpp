#include "cpu.hpp"

namespace {} // namespace

uint16_t CPU::read16( Operand_t register_ ) {
    auto index = static_cast<size_t>( register_ );
    return registers[index] | static_cast<uint16_t>( registers[index + 1] << 8 );
}

void CPU::write16( Operand_t register_, uint16_t value ) {
    registers[static_cast<size_t>( register_ )] = static_cast<uint8_t>( value );
    registers[static_cast<size_t>( register_ ) + 1] = static_cast<uint8_t>( value >> 8 );
}
