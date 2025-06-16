#include "core/cpu.hpp"

// Operand order is target first, source next
CoreCpu::MicroOperations_t CoreCpu::decode( int a ) {
    using enum MicroOperationType_t;
    // first check instructions without different operand variants
    const auto opcode = mem.read( PC++ );
    switch( opcode ) {
    //block 0
    case 0x0:
        return { { NOP } }; //NOP
    case 0x10:
        return { { STOP } }; // STOP
    case 0x08:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_SPL_TO_pWZ, LD_SPH_TO_pWZ, NOP } }; // LD pIMM16, SP
    case 0x07:
        return { { RLCA } };
    case 0x0F:
        return { { RRCA } };
    case 0x17:
        return { { RLA } };
    case 0x1F:
        return { { RRA } };
    case 0x27:
        return { { DAA } };
    case 0x2F:
        return { { CPL } };
    case 0x37:
        return { { SCF } };
    case 0x3F:
        return { { CCF } };
    case 0x18:
        return { { LD_IMM_TO_Z, ALU_CALC_RELATIVE_JUMP, IDU_LD_WZ_PLUS_1_TO_PC } }; // JR IMM8
    //block 1
    case 0x76:
        return { { HALT } }; // HALT
    //block 3
    //arithmetic
    case 0xC6:
        return { { LD_IMM_TO_Z, ALU_ADD_Z_TO_A } }; // ADD IMM8
    case 0xCE:
        return { { LD_IMM_TO_Z, ALU_ADD_Z_AND_C_TO_A } }; // ADC IMM8
    case 0xD6:
        return { { LD_IMM_TO_Z, ALU_SUB_Z_FROM_A } }; // SUB IMM8
    case 0xDE:
        return { { LD_IMM_TO_Z, ALU_SUB_Z_AND_C_FROM_A } }; // SBC IMM8
    case 0xE6:
        return { { LD_IMM_TO_Z, ALU_AND_AZ } }; // AND IMM8
    case 0xEE:
        return { { LD_IMM_TO_Z, ALU_XOR_AZ } }; // XOR IMM8
    case 0xF6:
        return { { LD_IMM_TO_Z, ALU_OR_AZ } }; // OR IMM8
    case 0xFE:
        return { { LD_IMM_TO_Z, ALU_CP_AZ } }; // CP IMM8
    case 0xE8:
        return { { LD_IMM_TO_Z, ALU_ADD_SPL_TO_Z, ALU_SPH_PLUS_CADJ_TO_W, LD_WZ_TO_SP } }; // ADD IMM8s
    //control flow
    case 0xC9:
        return { { LD_SPL_TO_Z, LD_SPH_TO_W, LD_WZ_TO_PC, NOP } }; // RET
    case 0xD9:
        return { { LD_SPL_TO_Z, LD_SPH_TO_W, LD_WZ_TO_PC_AND_ENABLE_IME, NOP } }; // RETI
    case 0xC3:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_WZ_TO_PC, NOP } }; // JP IMM16
    case 0xE9:
        return { { JP_TO_pHL } }; // JP pHL
    case 0xCD:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, SP_DEC, LD_PCH_TO_SP, LD_PCL_TO_SP, NOP } }; // CALL IMM16
    //the rest
    case 0xCB:
        // return decodeCB( opcode );
    case 0xE2:
        return { { LD_A_TO_FF00_PLUS_C, NOP } }; //LDH pC, A
    case 0xE0:
        return { { LD_IMM_TO_Z, LD_A_TO_FF00_PLUS_Z, NOP } }; // LDH IMM8, A
    case 0xEA:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_A_TO_WZ, NOP } }; // LD pIMM16, A
    case 0xF2:
        return { { LD_FF00_PLUS_C_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LDH A, pC
    case 0xF0:
        return { { LD_IMM_TO_Z, LD_FF00_PLUS_Z_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LDH A, IMM8
    case 0xFA:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_WZ_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LD A, pIMM16
    case 0xF8:
        return { { LD_IMM_TO_Z, ALU_SPL_PLUS_Z_TO_L, ALU_SPH_PLUS_CADJ_TO_H } }; // LD HL, SP+IMM8s
    case 0xF9:
        return { { LD_HL_TO_SP, NOP } }; // LD SP, HL
    case 0xF3:
        return { { DI } };
    case 0xFB:
        return { { EI } };
    }

    switch( 0x3 & ( opcode >> 6 ) ) { // leave only 2 most significant bits
    case 0x0:
        return decodeBlock0( opcode );
    case 0x1: {
        const auto dest = static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) );
        const auto src  = static_cast<Operand_t>( 0x7 & opcode );

        return { opcode, OperationType_t::LD, getR8orPHLType( dest ), dest, getR8orPHLType( src ), src };
    }
    case 0x2:
        return decodeBlock2( opcode );
    case 0x3:
        return decodeBlock3( opcode );
    }
    return { { INVALID } };
}

CoreCpu::MicroOperations_t CoreCpu::decodeBlock0( const uint8_t opcode ) {
    using enum MicroOperationType_t;
    //count from 0
    const auto operandR8  = static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) );
    const auto operandR16 = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
    switch( 0x7 & opcode ) {
    case 0x0:
        // Return longer version ( branch taken ), which can be shorten later
        return { { { COND_CHECK_IMM8e, static_cast<Operand_t>( 0x3 & ( opcode >> 3 ) ) },
                   ALU_CALC_RELATIVE_JUMP,
                   IDU_LD_WZ_PLUS_1_TO_PC } }; // JR COND, IMM8
    case 0x4:
        if( isPHL( operandR8 ) )
            return { { LD_pHL_TO_Z, ALU_LD_Z_PLUS_1_TO_pHL, NOP } }; // INC pHL
        else
            return { { { INC_R8, operandR8 } } }; // INC R8
    case 0x5:
        if( isPHL( operandR8 ) )
            return { { LD_pHL_TO_Z, ALU_LD_Z_MINUS_1_TO_pHL, NOP } }; // DEC pHL
        else
            return { { { DEC_R8, operandR8 } } }; // DEC R8
    case 0x6:
        if( isPHL( operandR8 ) )
            return { { LD_pHL_TO_Z, LD_Z_TO_pHL, NOP } }; // LD pHL, IMM8
        else
            return { { LD_IMM_TO_Z, { LD_Z_TO_R8, operandR8 } } }; // LD R8, IMM8
    }

    switch( 0xF & opcode ) {
    case 0x1:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, { LD_WZ_TO_R16, operandR16 } } }; // LD R16, IMM16
    case 0x2:
        return { { { LD_R16_MEM_TO_A, operandR16 }, NOP } }; // LD R16MEM, A
    case 0x3:
        return { { { IDU_INC_R16, operandR16 }, NOP } }; // INC R16
    case 0x9:
        return { { { ALU_ADD_LSB_R16_TO_L, operandR16 },
                   { ALU_ADD_CMSB_R16_TO_H, operandR16 } } }; // ADD HL, R16
    case 0xA:
        return { { { LD_R16_MEM_TO_Z, operandR16 }, { LD_Z_TO_R8, Operand_t::a } } }; // LD A, R16MEM
    case 0xB:
        return { { { IDU_DEC_R16, operandR16 }, NOP } }; // DEC R16
    }
    return { { INVALID } };
}

CoreCpu::Operation_t CoreCpu::decodeBlock2( const uint8_t opcode ) {
    const auto r8     = static_cast<Operand_t>( 0x7 & opcode );
    const auto r8Type = getR8orPHLType( r8 );
    auto opType       = OperationType_t::INVALID;
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
    const auto r16stk    = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
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

CoreCpu::MicroOperations_t CoreCpu::decodeCB( const uint8_t opcodeFirstByte ) {
    const auto opcodeSecondByte = mem.read( PC++ );
    const auto opcode           = static_cast<uint16_t>( opcodeFirstByte | opcodeSecondByte << 8 );
    const auto r8               = static_cast<Operand_t>( opcodeSecondByte & 0x7 );
    const auto r8Type           = getR8orPHLType( r8 );
    const auto b3index          = static_cast<Operand_t>( 0x7 & ( opcodeSecondByte >> 3 ) );
    switch( 0x3 & ( opcodeSecondByte >> 6 ) ) {
    case 0x0:
        switch( 0x7 & opcodeSecondByte ) {
        case 0x0:
            if( isPHL( operandR8 ) )
                return { { NOP, LD_pHL_TO_Z, RLC_Z, NOP } }; // RLC pHL
            else
                return { { NOP, RLC_R8 } }; // RLC R8
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


void CoreCpu::execute( const MicroOperation_t mop ) {
    switch( mop.type ) {
        using enum MicroOperationType_t;
    case NOP:
        return;
    }
}
