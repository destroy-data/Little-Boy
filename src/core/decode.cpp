#include "core/cpu.hpp"
#include <utility>

// source: https://gbdev.io/pandocs/CPU_Instruction_Set.html

CoreCpu::Operation_t CoreCpu::decode() {
    // first check instructions without different operand variants
    const auto opcode = mem.read( PC++ );
    switch( opcode ) {
    //block 0
    case 0x0:
        return { opcode, OperationType_t::NOP };
    case 0x10:
        return { opcode, OperationType_t::STOP };
    case 0x08:
        return { opcode, OperationType_t::LD, OperandType_t::pIMM16, {}, OperandType_t::R16, Operand_t::sp };
    case 0x07:
        return { opcode, OperationType_t::RLCA };
    case 0x0F:
        return { opcode, OperationType_t::RRCA };
    case 0x17:
        return { opcode, OperationType_t::RLA };
    case 0x1F:
        return { opcode, OperationType_t::RRA };
    case 0x27:
        return { opcode, OperationType_t::DAA };
    case 0x2F:
        return { opcode, OperationType_t::CPL };
    case 0x37:
        return { opcode, OperationType_t::SCF };
    case 0x3F:
        return { opcode, OperationType_t::CCF };
    case 0x18:
        return { opcode, OperationType_t::JR, OperandType_t::IMM8 };
    //block 1
    case 0x76:
        return { opcode, OperationType_t::HALT };
    //block 3
    //arithmetic
    case 0xC6:
        return { opcode, OperationType_t::ADD, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xCE:
        return { opcode, OperationType_t::ADC, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xD6:
        return { opcode, OperationType_t::SUB, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xDE:
        return { opcode, OperationType_t::SBC, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xE6:
        return { opcode, OperationType_t::AND, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xEE:
        return { opcode, OperationType_t::XOR, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xF6:
        return { opcode, OperationType_t::OR, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xFE:
        return { opcode, OperationType_t::CP, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xE8:
        return { opcode, OperationType_t::ADD, OperandType_t::R16, Operand_t::sp, OperandType_t::IMM8 };
    //control flow
    case 0xC9:
        return { opcode, OperationType_t::RET };
    case 0xD9:
        return { opcode, OperationType_t::RETI };
    case 0xC3:
        return { opcode, OperationType_t::JP, OperandType_t::IMM16 };
    case 0xE9:
        return { opcode, OperationType_t::JP, OperandType_t::R16, Operand_t::hl };
    case 0xCD:
        return { opcode, OperationType_t::CALL, OperandType_t::IMM16 };
    //the rest
    case 0xCB:
        return decodeCB( opcode );
    // SOURCES_DISAGREE
    // rgbds.gbdev.io/docs/v0.9.1/gbz80.7 says that there are n16 operands used
    // gbdev.io/pandocs/CPU_Instruction_Set.html says that n8 is used
    // gekkio.fi/files/gb-docs/gbctr.pdf also says that n8 is used
    // I'll go with n8
    case 0xE2:
        return { opcode,       OperationType_t::LDH, OperandType_t::FF00_PLUS_R8,
                 Operand_t::c, OperandType_t::R8,    Operand_t::a };
    case 0xE0:
        return { opcode, OperationType_t::LDH, OperandType_t::IMM8, {}, OperandType_t::R8, Operand_t::a };
    case 0xEA:
        return { opcode, OperationType_t::LD, OperandType_t::pIMM16, {}, OperandType_t::R8, Operand_t::a };
    case 0xF2:
        return { opcode,       OperationType_t::LDH,        OperandType_t::R8,
                 Operand_t::a, OperandType_t::FF00_PLUS_R8, Operand_t::c };
    case 0xF0:
        return { opcode, OperationType_t::LDH, OperandType_t::R8, Operand_t::a, OperandType_t::IMM8 };
    case 0xFA:
        return { opcode, OperationType_t::LD, OperandType_t::R8, Operand_t::a, OperandType_t::pIMM16 };
    case 0xF8:
        return { opcode, OperationType_t::LD, OperandType_t::R16, Operand_t::hl, OperandType_t::SP_PLUS_IMM8 };
    case 0xF9:
        return { opcode,        OperationType_t::LD, OperandType_t::R16,
                 Operand_t::sp, OperandType_t::R16,  Operand_t::hl };
    case 0xF3:
        return { opcode, OperationType_t::DI };
    case 0xFB:
        return { opcode, OperationType_t::EI };
    }

    switch( 0x3 & ( opcode >> 6 ) ) { // leave only 2 most significant bits
    case 0x0:
        return decodeBlock0( opcode );
    case 0x1: {
        const auto dest = static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) );
        const auto src = static_cast<Operand_t>( 0x7 & opcode );

        return { opcode, OperationType_t::LD, getR8orPHLType( dest ), dest, getR8orPHLType( src ), src };
    }
    case 0x2:
        return decodeBlock2( opcode );
    case 0x3:
        return decodeBlock3( opcode );
    }
    return { opcode, OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeBlock0( const uint8_t opcode ) {
    //count from 0
    const auto operandR8 = static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) );
    const auto operandR16 = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
    switch( 0x7 & opcode ) {
    case 0x0:
        return { opcode, OperationType_t::JR, OperandType_t::COND,
                 static_cast<Operand_t>( 0x3 & ( opcode >> 3 ) ), OperandType_t::IMM8 };
    case 0x4:
        return { opcode, OperationType_t::INC, getR8orPHLType( operandR8 ), operandR8 };
    case 0x5:
        return { opcode, OperationType_t::DEC, getR8orPHLType( operandR8 ), operandR8 };
    case 0x6:
        return { opcode, OperationType_t::LD, getR8orPHLType( operandR8 ), operandR8, OperandType_t::IMM8 };
    }

    switch( 0xF & opcode ) {
    case 0x1:
        return { opcode, OperationType_t::LD, OperandType_t::R16, operandR16, OperandType_t::IMM16 };
    case 0x2:
        return { opcode,     OperationType_t::LD, OperandType_t::R16MEM,
                 operandR16, OperandType_t::R8,   Operand_t::a };
    case 0x3:
        return { opcode, OperationType_t::INC, OperandType_t::R16, operandR16 };
    case 0x9:
        return { opcode,        OperationType_t::ADD, OperandType_t::R16,
                 Operand_t::hl, OperandType_t::R16,   operandR16 };
    case 0xA:
        return { opcode,       OperationType_t::LD,   OperandType_t::R8,
                 Operand_t::a, OperandType_t::R16MEM, operandR16 };
    case 0xB:
        return { opcode, OperationType_t::DEC, OperandType_t::R16, operandR16 };
    }
    return Operation_t { opcode, OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeBlock2( const uint8_t opcode ) {
    const auto r8 = static_cast<Operand_t>( 0x7 & opcode );
    const auto r8Type = getR8orPHLType( r8 );
    auto opType = OperationType_t::INVALID;
    switch( 0x7 & ( opcode >> 3 ) ) {
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
    return { opcode, opType, OperandType_t::R8, Operand_t::a, r8Type, r8 };
}


CoreCpu::Operation_t CoreCpu::decodeBlock3( const uint8_t opcode ) {
    const auto condition = static_cast<Operand_t>( 0x3 & ( opcode >> 3 ) );
    const auto r16stk = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
    switch( 0x7 & opcode ) {
    case 0x0:
        return { opcode, OperationType_t::RET, OperandType_t::COND, condition };
    case 0x2:
        return { opcode, OperationType_t::JP, OperandType_t::COND, condition, OperandType_t::IMM16 };
    case 0x4:
        return { opcode, OperationType_t::CALL, OperandType_t::COND, condition, OperandType_t::IMM16 };
    case 0x7:
        return { opcode, OperationType_t::RST, OperandType_t::TGT3,
                 static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) ) };
    case 0x1:
        return { opcode, OperationType_t::POP, OperandType_t::R16STK, r16stk };
    case 0x5:
        return { opcode, OperationType_t::PUSH, OperandType_t::R16STK, r16stk };
    }
    return { opcode, OperationType_t::INVALID };
}

CoreCpu::Operation_t CoreCpu::decodeCB( const uint8_t opcodeFirstByte ) {
    const auto opcodeSecondByte = mem.read( PC++ );
    const auto opcode = static_cast<uint16_t>( opcodeFirstByte | opcodeSecondByte << 8 );
    const auto r8 = static_cast<Operand_t>( opcodeSecondByte & 0x7 );
    const auto r8Type = getR8orPHLType( r8 );
    const auto b3index = static_cast<Operand_t>( 0x7 & ( opcodeSecondByte >> 3 ) );
    switch( 0x3 & ( opcodeSecondByte >> 6 ) ) {
    case 0x0:
        switch( 0x7 & opcodeSecondByte ) {
        case 0x0:
            return { opcode, OperationType_t::RLC, r8Type, r8 };
        case 0x1:
            return { opcode, OperationType_t::RRC, r8Type, r8 };
        case 0x2:
            return { opcode, OperationType_t::RL, r8Type, r8 };
        case 0x3:
            return { opcode, OperationType_t::RR, r8Type, r8 };
        case 0x4:
            return { opcode, OperationType_t::SLA, r8Type, r8 };
        case 0x5:
            return { opcode, OperationType_t::SRA, r8Type, r8 };
        case 0x6:
            return { opcode, OperationType_t::SWAP, r8Type, r8 };
        case 0x7:
            return { opcode, OperationType_t::SRL, r8Type, r8 };
        default:
            std::unreachable();
        }

    case 0x1:
        return { opcode, OperationType_t::BIT, OperandType_t::BIT_INDEX, b3index, r8Type, r8 };
    case 0x2:
        return { opcode, OperationType_t::RES, OperandType_t::BIT_INDEX, b3index, r8Type, r8 };
    case 0x3:
        return { opcode, OperationType_t::SET, OperandType_t::BIT_INDEX, b3index, r8Type, r8 };
    default:
        std::unreachable();
    }
}
