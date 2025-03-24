#include "core/cpu.hpp"
#include <utility>

CoreCpu::Operation_t CoreCpu::decode() {
    // first check instructions without different operand variants
    switch( mem.read( PC ) ) {
    //block 0
    case 0x0:
        return { OperationType_t::NOP };
    case 0x10:
        return { OperationType_t::STOP };
    case 0x08:
        return {
                OperationType_t::LD, OperandType_t::pIMM16, {}, OperandType_t::R16, Operand_t::sp };
    case 0x07:
        return { OperationType_t::RLCA };
    case 0x0F:
        return { OperationType_t::RRCA };
    case 0x17:
        return { OperationType_t::RLA };
    case 0x1F:
        return { OperationType_t::RRA };
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
    case 0xE8:
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
        return { OperationType_t::LDH, OperandType_t::IMM8, {}, OperandType_t::R8, Operand_t::a };
    case 0xEA:
        return { OperationType_t::LD, OperandType_t::pIMM16, {}, OperandType_t::R8, Operand_t::a };
    case 0xF2:
        return { OperationType_t::LDH, OperandType_t::R8, Operand_t::a, OperandType_t::R8,
                 Operand_t::c };
    case 0xF0:
        return { OperationType_t::LDH, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xFA:
        return { OperationType_t::LD, OperandType_t::R8, Operand_t::a, OperandType_t::pIMM16 };
    case 0xF8:
        return { OperationType_t::LD, OperandType_t::R16, Operand_t::hl,
                 OperandType_t::SP_PLUS_IMM8 };
    case 0xF9:
        return { OperationType_t::LD, OperandType_t::R16, Operand_t::sp, OperandType_t::R16,
                 Operand_t::hl };
    case 0xF3:
        return { OperationType_t::DI };
    case 0xFB:
        return { OperationType_t::EI };
    }

    switch( 0x3 & ( mem.read( PC ) >> 6 ) ) { // leave only 2 most significant bits
    case 0x0:
        return decodeBlock0();
    case 0x1: {
        auto dest = static_cast<Operand_t>( 0x7 & ( mem.read( PC ) >> 3 ) );
        auto src = static_cast<Operand_t>( 0x7 & mem.read( PC ) );
        return { OperationType_t::LD, OperandType_t::R8, dest, OperandType_t::R8, src };
    }
    case 0x2:
        return decodeBlock2();
    case 0x3:
        return decodeBlock3();
    }
    return { OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeBlock0() {
    //count from 0
    auto bits345 = static_cast<Operand_t>( 0x7 & ( mem.read( PC ) >> 3 ) );
    auto bits45 = static_cast<Operand_t>( 0x3 & ( mem.read( PC ) >> 4 ) );
    switch( 0x7 & mem.read( PC ) ) {
    case 0x0:
        return { OperationType_t::JR, OperandType_t::COND,
                 static_cast<Operand_t>( 0x3 & ( mem.read( PC ) >> 3 ) ), OperandType_t::IMM8 };
    case 0x4:
        return { OperationType_t::INC, OperandType_t::R8, bits345 };
    case 0x5:
        return { OperationType_t::DEC, OperandType_t::R8, bits345 };
    case 0x6:
        return { OperationType_t::LD, OperandType_t::R8, bits345, OperandType_t::IMM8 };
    }
    switch( 0xF & mem.read( PC ) ) {
    case 0x1:
        return { OperationType_t::LD, OperandType_t::R16, bits45, OperandType_t::IMM16 };
    case 0x2:
        return { OperationType_t::LD, OperandType_t::R16MEM, bits45, OperandType_t::R8,
                 Operand_t::a };
    case 0x3:
        return { OperationType_t::INC, OperandType_t::R16, bits45 };
    case 0x9:
        return { OperationType_t::ADD, OperandType_t::R16, Operand_t::hl, OperandType_t::R16,
                 bits45 };
    case 0xA:
        return { OperationType_t::LD, OperandType_t::R8, Operand_t::a, OperandType_t::R16MEM,
                 bits45 };
    case 0xB:
        return { OperationType_t::DEC, OperandType_t::R16, bits45 };
    }
    return Operation_t { OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeBlock2() {
    auto r8 = static_cast<Operand_t>( 0x7 & mem.read( PC ) );
    auto opType = OperationType_t::INVALID;
    switch( 0x7 & ( mem.read( PC ) >> 3 ) ) {
    case 0x0:
        opType = OperationType_t::ADD;
        break;
    case 0x1:
        opType = OperationType_t::ADC;
        break;
    case 0x2:
        opType = OperationType_t::SUB;
        break;
    case 0x3:
        opType = OperationType_t::SBC;
        break;
    case 0x4:
        opType = OperationType_t::AND;
        break;
    case 0x5:
        opType = OperationType_t::XOR;
        break;
    case 0x6:
        opType = OperationType_t::OR;
        break;
    case 0x7:
        opType = OperationType_t::CP;
    }
    return { opType, OperandType_t::R8, Operand_t::a, OperandType_t::R8, r8 };
}


CoreCpu::Operation_t CoreCpu::decodeBlock3() {
    auto condition = static_cast<Operand_t>( 0x3 & ( mem.read( PC ) >> 3 ) );
    auto r16stk = static_cast<Operand_t>( 0x3 & ( mem.read( PC ) >> 4 ) );
    switch( 0x7 & mem.read( PC ) ) {
    case 0x0:
        return { OperationType_t::RET, OperandType_t::COND, condition };
    case 0x2:
        return { OperationType_t::JP, OperandType_t::COND, condition, OperandType_t::IMM16 };
    case 0x4:
        return { OperationType_t::CALL, OperandType_t::COND, condition, OperandType_t::IMM16 };
    case 0x7:
        return { OperationType_t::RST, OperandType_t::TGT3,
                 static_cast<Operand_t>( 0x7 & ( mem.read( PC ) >> 3 ) ) };
    case 0x1:
        return { OperationType_t::POP, OperandType_t::R16STK, r16stk };
    case 0x5:
        return { OperationType_t::PUSH, OperandType_t::R16STK, r16stk };
    }
    return { OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeCB() {
    auto r8 = static_cast<Operand_t>( mem.read( PC ) & 0x7 );
    auto b3index = static_cast<Operand_t>( 0x7 & ( mem.read( PC ) >> 3 ) );
    switch( 0x3 & ( mem.read( PC ) >> 6 ) ) {
    case 0x0:
        switch( 0x7 & mem.read( PC ) ) {
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
        return { OperationType_t::BIT, OperandType_t::BIT_INDEX, b3index, OperandType_t::R8, r8 };
    case 0x2:
        return { OperationType_t::RES, OperandType_t::BIT_INDEX, b3index, OperandType_t::R8, r8 };
    case 0x3:
        return { OperationType_t::SET, OperandType_t::BIT_INDEX, b3index, OperandType_t::R8, r8 };
    default:
        std::unreachable();
    }
}
