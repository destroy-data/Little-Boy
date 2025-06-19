#include "core/cpu.hpp"
#include <utility>

// TODO adding signed IMM8 - check this
// TODO check LD operations regarding SP

using enum CoreCpu::MicroOperationType_t;
// Operand order is target first, source next
CoreCpu::MicroOperations_t CoreCpu::decode() {
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
        return { { LD_IMM_TO_Z, ALU_ADC_Z_TO_A } }; // ADC IMM8
    case 0xD6:
        return { { LD_IMM_TO_Z, ALU_SUB_Z_FROM_A } }; // SUB IMM8
    case 0xDE:
        return { { LD_IMM_TO_Z, ALU_SBC_Z_FROM_A } }; // SBC IMM8
    case 0xE6:
        return { { LD_IMM_TO_Z, ALU_A_AND_Z } }; // AND IMM8
    case 0xEE:
        return { { LD_IMM_TO_Z, ALU_A_XOR_Z } }; // XOR IMM8
    case 0xF6:
        return { { LD_IMM_TO_Z, ALU_A_OR_Z } }; // OR IMM8
    case 0xFE:
        return { { LD_IMM_TO_Z, ALU_A_CP_Z } }; // CP IMM8
    case 0xE8:
        return { { LD_IMM_TO_Z, ALU_ADD_SPL_TO_Z, ALU_SPH_ADC_ADJ_TO_W, LD_WZ_TO_SP } }; // ADD SP, IMM8s
    //control flow
    case 0xC9:
        return { { POP_SP_TO_Z, POP_SP_TO_W, LD_WZ_TO_PC, NOP } }; // RET
    case 0xD9:
        return { { POP_SP_TO_Z, POP_SP_TO_W, LD_WZ_TO_PC__ENABLE_IME, NOP } }; // RETI
    case 0xC3:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_WZ_TO_PC, NOP } }; // JP IMM16
    case 0xE9:
        return { { JP_TO_HL } }; // JP pHL
    case 0xCD:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, SP_DEC, LD_PCH_TO_SP, LD_PCL_TO_SP, NOP } }; // CALL IMM16
    //the rest
    case 0xCB:
        return decodeCB();
    case 0xE2:
        return { { LD_A_TO_FF00_PLUS_C, NOP } }; //LDH pC, A
    case 0xE0:
        return { { LD_IMM_TO_Z, LD_A_TO_FF00_PLUS_Z, NOP } }; // LDH IMM8, A
    case 0xEA:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_A_TO_pWZ, NOP } }; // LD pIMM16, A
    case 0xF2:
        return { { LD_FF00_PLUS_C_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LDH A, pC
    case 0xF0:
        return { { LD_IMM_TO_Z, LD_FF00_PLUS_Z_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LDH A, IMM8
    case 0xFA:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, LD_pWZ_TO_Z, { LD_Z_TO_R8, Operand_t::a } } }; // LD A, pIMM16
    case 0xF8:
        return { { LD_IMM_TO_Z, ALU_SPL_PLUS_Z_TO_L, ALU_SPH_ADC_ADJ_TO_H } }; // LD HL, SP+IMM8s
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

        if( ! isPHL( dest ) && ! isPHL( src ) )
            return { { { LD_R8_TO_R8, dest, src } } }; // LD R8, R8
        if( isPHL( src ) )
            return { { LD_pHL_TO_Z, { LD_Z_TO_R8, dest } } }; // LD R8, pHL
        if( isPHL( dest ) )
            return { { { LD_R8_TO_pHL, src }, NOP } }; // LD pHL, R8

        return { { INVALID } };
    }
    case 0x2:
        return decodeBlock2( opcode );
    case 0x3:
        return decodeBlock3( opcode );
    }
    return { { INVALID } };
}


CoreCpu::MicroOperations_t CoreCpu::decodeBlock0( const uint8_t opcode ) {
    //count from 0
    const auto r8  = static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) );
    const auto r16 = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
    switch( 0x7 & opcode ) {
    case 0x0:
        // Return longer version ( branch taken ), which can be shorten later
        return { { { COND_CHECK__LD_IMM_TO_Z, static_cast<Operand_t>( 0x3 & ( opcode >> 3 ) ) },
                   ALU_CALC_RELATIVE_JUMP,
                   IDU_LD_WZ_PLUS_1_TO_PC } }; // JR COND, IMM8
    case 0x4:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_LD_Z_PLUS_1_TO_pHL, NOP } }; // INC pHL
        else
            return { { { INC_R8, r8 } } }; // INC R8
    case 0x5:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_LD_Z_MINUS_1_TO_pHL, NOP } }; // DEC pHL
        else
            return { { { DEC_R8, r8 } } }; // DEC R8
    case 0x6:
        if( isPHL( r8 ) )
            return { { LD_IMM_TO_Z, LD_Z_TO_pHL, NOP } }; // LD pHL, IMM8
        else
            return { { LD_IMM_TO_Z, { LD_Z_TO_R8, r8 } } }; // LD R8, IMM8
    }

    switch( 0xF & opcode ) {
    case 0x1:
        return { { LD_IMM_TO_Z, LD_IMM_TO_W, { LD_WZ_TO_R16, r16 } } }; // LD R16, IMM16
    case 0x2:
        return { { { LD_A_TO_R16_MEM, r16 }, NOP } }; // LD R16MEM, A
    case 0x3:
        return { { { IDU_INC_R16, r16 }, NOP } }; // INC R16
    case 0x9:
        return { { { ALU_ADD_LSB_R16_TO_L, r16 }, { ALU_ADC_MSB_R16_TO_H, r16 } } }; // ADD HL, R16
    case 0xA:
        return { { { LD_R16_MEM_TO_Z, r16 }, { LD_Z_TO_R8, Operand_t::a } } }; // LD A, R16MEM
    case 0xB:
        return { { { IDU_DEC_R16, r16 }, NOP } }; // DEC R16
    }
    return { { INVALID } };
}


CoreCpu::MicroOperations_t CoreCpu::decodeBlock2( const uint8_t opcode ) {
    const auto r8 = static_cast<Operand_t>( 0x7 & opcode );
    switch( 0x7 & ( opcode >> 3 ) ) {
    case 0x0:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_ADD_Z_TO_A } }; // ADD pHL
        else
            return { { { ALU_ADD_R8_TO_A, r8 } } }; // ADD r8
    case 0x1:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_ADC_Z_TO_A } }; // ADC pHL
        else
            return { { { ALU_ADC_R8_TO_A, r8 } } }; // ADC r8
    case 0x2:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_SUB_Z_FROM_A } }; // SUB pHL
        else
            return { { { ALU_SUB_R8_FROM_A, r8 } } }; // SUB r8
    case 0x3:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_SBC_Z_FROM_A } }; // SBC pHL
        else
            return { { { ALU_SBC_R8_FROM_A, r8 } } }; // SBC r8
    case 0x4:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_A_AND_Z } }; // AND pHL
        else
            return { { { ALU_A_AND_R8, r8 } } }; // AND r8
    case 0x5:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_A_XOR_Z } }; // XOR pHL
        else
            return { { { ALU_A_XOR_R8, r8 } } }; // XOR r8
    case 0x6:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_A_OR_Z } }; // OR pHL
        else
            return { { { ALU_A_OR_R8, r8 } } }; // OR r8
    case 0x7:
        if( isPHL( r8 ) )
            return { { LD_pHL_TO_Z, ALU_A_CP_Z } }; // CP pHL
        else
            return { { { ALU_CP_A_R8, r8 } } }; // CP r8
    }
    return { { INVALID } };
}


CoreCpu::MicroOperations_t CoreCpu::decodeBlock3( const uint8_t opcode ) {
    const auto condition = static_cast<Operand_t>( 0x3 & ( opcode >> 3 ) );
    const auto r16stk    = static_cast<Operand_t>( 0x3 & ( opcode >> 4 ) );
    switch( 0x7 & opcode ) {
    case 0x0:
        // Return longer version ( branch taken ), which can be shorten later
        return { { { CHECK_COND, condition }, POP_SP_TO_Z, POP_SP_TO_W, LD_WZ_TO_PC, NOP } }; // RET cc
    case 0x2:
        // Return longer version ( branch taken ), which can be shorten later
        return { { LD_IMM_TO_Z, { COND_CHECK__LD_IMM_TO_W, condition }, LD_WZ_TO_PC, NOP } }; // JP cc, IMM16
    case 0x4:
        // Return longer version ( branch taken ), which can be shorten later
        return { { LD_IMM_TO_Z,
                   COND_CHECK__LD_IMM_TO_W,
                   { IDU_DEC_R16, Operand_t::sp },
                   LD_PCH_TO_SP,
                   LD_PCL_TO_SP__LD_WZ_TO_PC,
                   NOP } }; // CALL cc, IMM16
    case 0x7:
        return { { { IDU_DEC_R16, Operand_t::sp },
                   LD_PCH_TO_SP,
                   { LD_PCL_TO_SP__LD_TGT3_TO_PC, static_cast<Operand_t>( 0x7 & ( opcode >> 3 ) ) },
                   NOP } }; // RST TGT3
    case 0x1:
        return { { POP_SP_TO_Z, POP_SP_TO_W, { LD_WZ_TO_R16STK, r16stk } } }; // POP R16STK
    case 0x5:
        return { { IDU_DEC_R16,
                   { PUSH_MSB_R16STK_TO_SP, r16stk },
                   { PUSH_LSB_R16STK_TO_SP, r16stk },
                   NOP } }; // PUSH R16STK
    }
    return { { INVALID } };
}


CoreCpu::MicroOperations_t CoreCpu::decodeCB() {
    const auto opcodeSecondByte = mem.read( PC );
    const auto r8               = static_cast<Operand_t>( opcodeSecondByte & 0x7 );
    const auto b3index          = static_cast<Operand_t>( 0x7 & ( opcodeSecondByte >> 3 ) );
    switch( 0x3 & ( opcodeSecondByte >> 6 ) ) {
    case 0x0:
        switch( 0x7 & opcodeSecondByte ) {
        case 0x0:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_RLC_Z_TO_pHL, NOP } }; // RLC pHL
            else
                return { { FETCH_SECOND_BYTE, { RLC_R8, r8 } } }; // RLC R8
        case 0x1:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_RRC_Z_TO_pHL, NOP } }; // RRC pHL
            else
                return { { FETCH_SECOND_BYTE, { RRC_R8, r8 } } }; // RRC R8
        case 0x2:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_RL_Z_TO_pHL, NOP } }; // RL pHL
            else
                return { { FETCH_SECOND_BYTE, { RL_R8, r8 } } }; // RL R8
        case 0x3:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_RR_Z_TO_pHL, NOP } }; // RR pHL
            else
                return { { FETCH_SECOND_BYTE, { RR_R8, r8 } } }; // RR R8
        case 0x4:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_SLA_Z_TO_pHL, NOP } }; // SLA pHL
            else
                return { { FETCH_SECOND_BYTE, { SLA_R8, r8 } } }; // SLA R8
        case 0x5:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_SRA_Z_TO_pHL, NOP } }; // SRA pHL
            else
                return { { FETCH_SECOND_BYTE, { SRA_R8, r8 } } }; // SRA R8
        case 0x6:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_SWAP_Z_TO_pHL, NOP } }; // SWAP pHL
            else
                return { { FETCH_SECOND_BYTE, { SWAP_R8, r8 } } }; // SRA R8
        case 0x7:
            if( isPHL( r8 ) )
                return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, LD_SRL_Z_TO_pHL, NOP } }; // SRL pHL
            else
                return { { FETCH_SECOND_BYTE, { SRL_R8, r8 } } }; // SRL R8
        default:
            std::unreachable();
        }

    case 0x1:
        if( isPHL( r8 ) )
            return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, { BIT_Z, b3index }, NOP } }; // BIT B3, pHL
        else
            return { { FETCH_SECOND_BYTE, { BIT_R8, b3index, r8 } } }; // BIT B3, R8
    case 0x2:
        if( isPHL( r8 ) )
            return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, { RES_pHL, b3index }, NOP } }; // RES B3, pHL
        else
            return { { FETCH_SECOND_BYTE, { RES_R8, b3index, r8 } } }; // RES B3, R8
    case 0x3:
        if( isPHL( r8 ) )
            return { { FETCH_SECOND_BYTE, LD_pHL_TO_Z, { SET_pHL, b3index }, NOP } }; // SET B3, pHL
        else
            return { { FETCH_SECOND_BYTE, { SET_R8, b3index, r8 } } }; // SET B3, R8
    default:
        std::unreachable();
    }
}
