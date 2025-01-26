#include "cpu.hpp"

namespace {} // namespace

uint16_t CPU::read16( Operand_t register_ ) {
    auto index = static_cast<size_t>( register_ );
    return registers[index] | ( registers[index + 1] << 8 );
}

void CPU::write16( Operand_t register_, uint16_t value ) {
    registers[static_cast<size_t>( register_ )] = static_cast<uint8_t>( value );
    registers[static_cast<size_t>( register_ ) + 1] = static_cast<uint8_t>( value >> 8 );
}

CPU::Operation_t CPU::decode() {
    // first check instructions without different operand variants
    switch( ROM[PC] ) {
    case 0:
        return Operation_t { OperationType_t::NOOP };
    case 0x10:
        return Operation_t { OperationType_t::STOP };
    case 0x08:
        return Operation_t { OperationType_t::LD,
                             OperandType_t::IMM16,
                             {},
                             OperandType_t::R16,
                             Operand_t::sp };
    case 0x07:
        return {};
    }

    switch( 0xC0 & ROM[PC] ) { // leave only 2 most significant bits
    case 0b0:
        return decodeBlock00();
    case 0b1:
        return decodeBlock01();
    case 0b10:
        return decodeBlock10();
    case 0b11:
        return decodeBlock11();
    }
}

CPU::Operation_t CPU::decodeBlock00() {
    if( !ROM[PC] )
        return Operation_t { OperationType_t::NOOP };
    //count from 0
    uint8_t _3LsbBits = 0b111 & ROM[PC];
    uint8_t _45bits = 0b110000 & ROM[PC];
    uint8_t _345bits = 0b111000 & ROM[PC];
    switch( _3LsbBits ) {
    case 0b0:
        switch( ROM[PC] ) {
        case 0b1000:
                //return Operation_t {OperationType_t::LD, OperandType_t::IMM16, VALUE1, OperandType_t::R16, Register_t::sp};
                ;
        case 0b11000:;
        case 0b10000:;
        default:;
        }
    case 0b1:
        break;
    }
    return Operation_t { OperationType_t::INVALID };
}