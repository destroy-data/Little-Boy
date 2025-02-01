#include "cpu.hpp"
#include <utility>

namespace {} // namespace

uint16_t CPU::read16( Operand_t register_ ) {
    auto index = static_cast<size_t>( register_ );
    return registers[index] | static_cast<uint16_t>( registers[index + 1] << 8 );
}

void CPU::write16( Operand_t register_, uint16_t value ) {
    registers[static_cast<size_t>( register_ )] = static_cast<uint8_t>( value );
    registers[static_cast<size_t>( register_ ) + 1] = static_cast<uint8_t>( value >> 8 );
}

CPU::Operation_t CPU::decode() {
    // first check instructions without different operand variants
    // RLC, RRC, RL and RR technically have different variants, but those are 0xCB prefixed
    switch( ROM[PC] ) {
    //block 0
    case 0x0:
        return { OperationType_t::NOOP };
    case 0x10:
        return { OperationType_t::STOP };
    case 0x08:
        return { OperationType_t::LD,
                 OperandType_t::pIMM16,
                 {},
                 OperandType_t::R16,
                 Operand_t::sp };
    case 0x07:
        return { OperationType_t::RLC, OperandType_t::R8, Operand_t::a };
    case 0x0F:
        return { OperationType_t::RRC, OperandType_t::R8, Operand_t::a };
    case 0x17:
        return { OperationType_t::RL, OperandType_t::R8, Operand_t::a };
    case 0x1F:
        return { OperationType_t::RR, OperandType_t::R8, Operand_t::a };
    case 0x27:
        return { OperationType_t::DAA };
    case 0x2F:
        return { OperationType_t::CPL };
    case 0x37:
        return { OperationType_t::SCF };
    case 0x3F:
        return { OperationType_t::CCF };
    case 0x18:
        return { OperationType_t::JR, OperandType_t::IMM8 };
    //block 1
    case 0x76:
        return { OperationType_t::HALT };
    //block 3
    //arithmetic
    case 0xC6:
        return { OperationType_t::ADD, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xCE:
        return { OperationType_t::ADC, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xD6:
        return { OperationType_t::SUB, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xDE:
        return { OperationType_t::SBC, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xE6:
        return { OperationType_t::AND, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xEE:
        return { OperationType_t::XOR, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xF6:
        return { OperationType_t::OR, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xFE:
        return { OperationType_t::CP, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xD8:
        return { OperationType_t::ADD, OperandType_t::R16, Operand_t::sp, OperandType_t::IMM8 };
    //control flow
    case 0xC9:
        return { OperationType_t::RET };
    case 0xD9:
        return { OperationType_t::RETI };
    case 0xC3:
        return { OperationType_t::JP, OperandType_t::IMM16 };
    case 0xE9:
        return { OperationType_t::JP, OperandType_t::R16, Operand_t::hl };
    case 0xCD:
        return { OperationType_t::CALL, OperandType_t::IMM16 };
    //the rest
    case 0xCB:
        PC++;
        return decodeCB();
    case 0xE2:
        return { OperationType_t::LDH, OperandType_t::R8, Operand_t::c, OperandType_t::R8,
                 Operand_t::a };
    case 0xE0:
        return { OperationType_t::LDH, OperandType_t::pIMM8, {}, OperandType_t::R8, Operand_t::a };
    case 0xEA:
        return { OperationType_t::LD, OperandType_t::pIMM16, {}, OperandType_t::R8, Operand_t::a };
    case 0xF2:
        return { OperationType_t::LDH, OperandType_t::R8, Operand_t::a, OperandType_t::R8,
                 Operand_t::c };
    case 0xF0:
        return { OperationType_t::LDH, OperandType_t::R8, Operand_t::a, OperandType_t::pIMM8 };
    case 0xFA:
        return { OperationType_t::LD, OperandType_t::R8, Operand_t::a, OperandType_t::pIMM16 };
    case 0xF8:
        //TODO
        break;
    case 0xF9:
        return { OperationType_t::LD, OperandType_t::R16, Operand_t::sp, OperandType_t::R16,
                 Operand_t::hl };
    case 0xF3:
        return { OperationType_t::DI };
    case 0xFB:
        return { OperationType_t::EI };
    }

    switch( 0x3 & ( ROM[PC] >> 6 ) ) { // leave only 2 most significant bits
    case 0x0:
        return decodeBlock0();
    case 0x1: {
        auto dest = static_cast<Operand_t>( 0x7 & ( ROM[PC] >> 3 ) );
        auto src = static_cast<Operand_t>( 0x7 & ROM[PC] );
        return { OperationType_t::LD, OperandType_t::R8, dest, OperandType_t::R8, src };
    }
    case 0x2:
        return decodeBlock2();
    case 0x3:
        return decodeBlock3();
    }
    return { OperationType_t::INVALID };
}

CPU::Operation_t CPU::decodeBlock0() {
    //count from 0
    auto bytes345 = static_cast<Operand_t>( 0x7 & ( ROM[PC] >> 3 ) );
    auto bytes45 = static_cast<Operand_t>( 0x3 & ( ROM[PC] >> 4 ) );
    switch( 0x7 & ROM[PC] ) {
    case 0x0:
        return { OperationType_t::JR, OperandType_t::COND,
                 static_cast<Operand_t>( 0x3 & ( ROM[PC] >> 3 ) ), OperandType_t::IMM8 };
    case 0x4:
        return { OperationType_t::INC, OperandType_t::R8, bytes345 };
    case 0x5:
        return { OperationType_t::DEC, OperandType_t::R8, bytes345 };
    case 0x6:
        return { OperationType_t::LD, OperandType_t::R8, bytes345, OperandType_t::IMM8 };
    }
    switch( 0xF & ROM[PC] ) {
    case 0x1:
        return { OperationType_t::LD, OperandType_t::R16, bytes45, OperandType_t::IMM16 };
    case 0x2:
        return { OperationType_t::LD, OperandType_t::R16MEM, bytes45, OperandType_t::R8,
                 Operand_t::a };
    case 0x3:
        return { OperationType_t::INC, OperandType_t::R16, bytes45 };
    case 0x9:
        return { OperationType_t::LD, OperandType_t::R16, Operand_t::hl, OperandType_t::R16,
                 bytes45 };
    case 0xA:
        return { OperationType_t::LD, OperandType_t::R8, Operand_t::a, OperandType_t::R16MEM,
                 bytes45 };
    case 0xB:
        return { OperationType_t::DEC, OperandType_t::R16, bytes45 };
    }
    return Operation_t { OperationType_t::INVALID };
}

CPU::Operation_t CPU::decodeCB() {
    auto r8 = static_cast<Operand_t>( ROM[PC] & 0x7 );
    auto b3index = static_cast<Operand_t>( 0x7 & ( ROM[PC] >> 3 ) );
    switch( 0x3 & ( ROM[PC] >> 6 ) ) {
    case 0x0:
        switch( 0x7 & ROM[PC] ) {
        case 0x0:
            return { OperationType_t::RLC, OperandType_t::R8, r8 };
        case 0x1:
            return { OperationType_t::RRC, OperandType_t::R8, r8 };
        case 0x2:
            return { OperationType_t::RL, OperandType_t::R8, r8 };
        case 0x3:
            return { OperationType_t::RR, OperandType_t::R8, r8 };
        case 0x4:
            return { OperationType_t::SLA, OperandType_t::R8, r8 };
        case 0x5:
            return { OperationType_t::SRA, OperandType_t::R8, r8 };
        case 0x6:
            return { OperationType_t::SWAP, OperandType_t::R8, r8 };
        case 0x7:
            return { OperationType_t::SRL, OperandType_t::R8, r8 };
        default:
            std::unreachable();
        }

    case 0x1:
        return { OperationType_t::BIT, OperandType_t::B3_INDEX, b3index, OperandType_t::R8, r8 };
    case 0x2:
        return { OperationType_t::RES, OperandType_t::B3_INDEX, b3index, OperandType_t::R8, r8 };
    case 0x3:
        return { OperationType_t::SET, OperandType_t::B3_INDEX, b3index, OperandType_t::R8, r8 };
    default:
        std::unreachable();
    }
}
