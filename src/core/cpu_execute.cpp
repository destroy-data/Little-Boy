#include "core/core_utils.hpp"
#include "core/cpu.hpp"
#include "core/logging.hpp"
#include <utility>

void CoreCpu::execute( MicroOperation_t mop ) {
    switch( mop.type ) {
        using enum MicroOperationType_t;
    case NOP:
        return;
    case STOP:
        //TODO
        return;
    case LD_IMM_TO_Z:
        Z = mem.read( PC++ );
        break;
    case LD_IMM_TO_W:
        W = mem.read( PC++ );
        break;
    case LD_SPL_TO_pWZ:
        mem.write( getWZ(), lsb( SP ) );
        setWZ( getWZ() + 1 );
        break;
    case LD_SPH_TO_pWZ:
        mem.write( getWZ(), uint8_t( SP >> 8 ) );
        break;
    case RLCA: {
        const uint8_t value = readR8( Operand_t::a );
        const bool cFlag    = value & bit( 7 );

        const uint8_t newValue = std::rotl( value, 1 );
        writeR8( Operand_t::a, newValue );
        setZNHCFlags( 0, 0, 0, cFlag );
    } break;
    case RRCA: {
        const uint8_t value = readR8( Operand_t::a );
        const bool cFlag    = value & 0x1;

        const uint8_t newValue = std::rotr( value, 1 );
        writeR8( Operand_t::a, newValue );
        setZNHCFlags( 0, 0, 0, cFlag );
    } break;
    case RLA: {
        const uint8_t value = readR8( Operand_t::a );
        const bool cFlag    = value & bit( 7 );

        const uint8_t newValue = static_cast<uint8_t>( ( value << 1 ) | getCFlag() );
        writeR8( Operand_t::a, newValue );
        setZNHCFlags( 0, 0, 0, cFlag );
    } break;
    case RRA: {
        const uint8_t value = readR8( Operand_t::a );
        const bool cFlag    = value & 0x1;

        const uint8_t newValue = static_cast<uint8_t>( ( getCFlag() << 7 ) | ( value >> 1 ) );
        writeR8( Operand_t::a, newValue );
        setZNHCFlags( 0, 0, 0, cFlag );
    } break;
    case DAA: {
        const auto registerA = readR8( Operand_t::a );
        uint8_t adjustment   = 0;
        bool cFlag           = getCFlag();
        if( getNFlag() ) {
            if( getHFlag() )
                adjustment += 6;
            if( getCFlag() )
                adjustment += 0x60;
            writeR8( Operand_t::a, registerA - adjustment );
        } else {
            if( getHFlag() || ( registerA & 0xF ) > 9 )
                adjustment += 6;
            if( getCFlag() || registerA > 0x99 ) {
                adjustment += 0x60;
                cFlag = true;
            }
            writeR8( Operand_t::a, registerA + adjustment );
        }
        setZNHCFlags( ! readR8( Operand_t::a ), getNFlag(), 0, cFlag );
    } break;
    case CPL:
        writeR8( Operand_t::a, ~readR8( Operand_t::a ) );
        setNFlag( true );
        setHFlag( true );
        break;
    case SCF:
        setZNHCFlags( getZFlag(), 0, 0, 1 );
        break;
    case CCF:
        setZNHCFlags( getZFlag(), 0, 0, ! getCFlag() );
        break;
    case ALU_CALC_RELATIVE_JUMP: {
        const uint16_t tmp = static_cast<uint16_t>( PC + int8_t( Z ) );
        Z                  = lsb( tmp );
        W                  = msb( tmp );
    } break;
    case IDU_LD_WZ_PLUS_1_TO_PC:
        PC = getWZ() + 1;
        break;
    case HALT: {
        //interruptMasterEnabled = false; // GBCTR says so, but does it make sense?
        halted = true;
        // TODO halt bug
    } break;
    case ALU_ADD_Z_TO_A:
        addToR8( Operand_t::a, Z );
        break;
    case ALU_ADD_Z_AND_C_TO_A:
        addToR8( Operand_t::a, Z + getCFlag() );
        break;
    case ALU_SUB_Z_FROM_A:
        subFromR8( Operand_t::a, Z );
        break;
    case ALU_SUB_Z_AND_C_FROM_A:
        subFromR8( Operand_t::a, Z + getCFlag() );
        break;
    case ALU_A_AND_Z: {
        const uint8_t result = readR8( Operand_t::a ) & Z;
        writeR8( Operand_t::a, result );
        setZNHCFlags( ! result, 0, 1, 0 );
    } break;
    case ALU_A_XOR_Z: {
        const uint8_t result = readR8( Operand_t::a ) ^ Z;
        writeR8( Operand_t::a, result );
        setZNHCFlags( ! result, 0, 0, 0 );
    } break;
    case ALU_A_OR_Z: {
        const uint8_t result = readR8( Operand_t::a ) | Z;
        writeR8( Operand_t::a, result );
        setZNHCFlags( ! result, 0, 0, 0 );
    } break;
    case ALU_A_CP_Z:
        subFromR8( Operand_t::a, Z, true );
        break;
    case ALU_ADD_SPL_TO_Z:
        //TODO ADJ
        Z = addU8ToU8( lsb( SP ), Z );
        break;
    case ALU_SPH_ADC_ADJ_TO_W:
        // TODO
        setZFlag( 0 );
        setNFlag( 0 );
        break;
    case LD_WZ_TO_SP:
        SP = getWZ();
        break;
    case POP_SP_TO_Z:
        Z = mem.read( SP++ );
        break;
    case POP_SP_TO_W:
        W = mem.read( SP++ );
        break;
    case LD_WZ_TO_PC:
        PC = getWZ();
        break;
    case LD_WZ_TO_PC__ENABLE_IME:
        PC                     = getWZ();
        interruptMasterEnabled = true;
        break;
    case JP_TO_HL:
        PC = readR16( Operand_t::hl );
        break;
    case SP_DEC:
        SP--;
        break;
    case LD_PCH_TO_SP:
        mem.write( SP--, msb( PC ) );
        break;
    case LD_PCL_TO_SP:
        mem.write( SP, lsb( PC ) );
        PC = getWZ();
        break;
    case LD_A_TO_FF00_PLUS_C:
        mem.write( 0xFF00 | readR8( Operand_t::c ), readR8( Operand_t::a ) );
        break;
    case LD_A_TO_FF00_PLUS_Z:
        mem.write( 0xFF00 | Z, readR8( Operand_t::a ) );
        break;
    case LD_A_TO_pWZ:
        mem.write( getWZ(), readR8( Operand_t::a ) );
        break;
    case LD_FF00_PLUS_C_TO_Z:
        Z = mem.read( 0xFF00 | readR8( Operand_t::c ) );
        break;
    case LD_Z_TO_R8:
        writeR8( mop.operand1, Z );
        break;
    case LD_FF00_PLUS_Z_TO_Z:
        Z = mem.read( 0xFF00 + Z );
        break;
    case LD_pWZ_TO_Z:
        Z = mem.read( getWZ() );
        break;
    case ALU_SPL_PLUS_Z_TO_LHL:
        writeR8( Operand_t::l, addU8ToU8( lsb( SP ), Z ) );
        //TODO ADJ
        break;
    case ALU_SPH_ADC_ADJ_TO_HHL:
        //TODO
        break;
    case LD_HL_TO_SP:
        SP = readR16( Operand_t::hl );
        break;
    case DI:
        interruptMasterEnabled = false;
        break;
    case EI:
        enableIMELater = true;
        break;
    case COND_CHECK__LD_IMM_TO_Z:
        lastConditionCheck = isConditionMet( mop.operand1 );
        Z                  = mem.read( PC++ );
        break;
    case INC_R8:
        break;
    case LD_pHL_TO_Z:
        break;
    case ALU_LD_Z_PLUS_1_TO_pHL:
        break;
    case ALU_LD_Z_MINUS_1_TO_pHL:
        break;
    case DEC_R8:
        break;
    case LD_Z_TO_pHL:
        break;
    case LD_WZ_TO_R16:
        break;
    case LD_R16_MEM_TO_A:
        break;
    case IDU_INC_R16:
        break;
    case ALU_ADD_LSB_R16_TO_L:
        break;
    case ALU_ADD_CMSB_R16_TO_H:
        break;
    case LD_R16_MEM_TO_Z:
        break;
    case IDU_DEC_R16:
        break;
    case LD_R8_TO_R8:
        break;
    case LD_R8_TO_pHL:
        break;
    case ALU_ADD_R8_TO_A:
        break;
    case ALU_ADC_Z_TO_A:
        break;
    case ALU_ADC_R8_TO_A:
        break;
    case ALU_SUB_R8_FROM_A:
        break;
    case ALU_SBC_Z_FROM_A:
        break;
    case ALU_SBC_R8_FROM_A:
        break;
    case ALU_A_AND_R8:
        break;
    case ALU_A_XOR_R8:
        break;
    case ALU_A_OR_R8:
        break;
    case ALU_CP_A_R8:
        break;
    case CHECK_COND:
        break;
    case COND_CHECK__LD_IMM_TO_W:
        break;
    case LD_PCL_TO_SP__LD_WZ_TO_PC:
        break;
    case LD_PCL_TO_SP__LD_TGT3_TO_PC:
        break;
    case LD_WZ_TO_R16STK:
        break;
    case PUSH_MSB_R16STK_TO_SP:
        break;
    case PUSH_LSB_R16STK_TO_SP:
        break;
    case FETCH_SECOND_BYTE:
        PC++;
        break;
    case LD_RLC_Z_TO_pHL: {
        const bool cFlag = Z & bit( 7 );
        Z                = std::rotl( Z, 1 );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case RLC_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & bit( 7 );
        const uint8_t newValue = std::rotl( value, 1 );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_RRC_Z_TO_pHL: {
        const bool cFlag = Z & 0x1;
        Z                = std::rotr( Z, 1 );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case RRC_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & 0x1;
        const uint8_t newValue = std::rotr( value, 1 );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_RL_Z_TO_pHL: {
        const bool cFlag = Z & bit( 7 );
        Z                = static_cast<uint8_t>( ( Z << 1 ) | getCFlag() );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case RL_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & bit( 7 );
        const uint8_t newValue = static_cast<uint8_t>( ( value << 1 ) | getCFlag() );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_RR_Z_TO_pHL: {
        const bool cFlag = Z & 0x1;
        Z                = static_cast<uint8_t>( ( getCFlag() << 7 ) | ( Z >> 1 ) );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case RR_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & 0x1;
        const uint8_t newValue = static_cast<uint8_t>( ( getCFlag() << 7 ) | ( value >> 1 ) );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_SLA_Z_TO_pHL: {
        const bool cFlag = Z & bit( 7 );
        Z                = Z << 1;
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case SLA_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & bit( 7 );
        const uint8_t newValue = value << 1;
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_SRA_Z_TO_pHL: {
        const bool cFlag = Z & 0x1;
        Z                = static_cast<uint8_t>( ( Z & bit( 7 ) ) | ( Z >> 1 ) );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case SRA_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & 0x1;
        const uint8_t newValue = static_cast<uint8_t>( ( value & bit( 7 ) ) | ( value >> 1 ) );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case LD_SWAP_Z_TO_pHL:
        Z = static_cast<uint8_t>( ( Z << 4 ) | ( Z >> 4 ) );
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, 0 );
        break;
    case SWAP_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const uint8_t newValue = static_cast<uint8_t>( ( value << 4 ) | ( value >> 4 ) );
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, 0 );
    } break;
    case LD_SRL_Z_TO_pHL: {
        const bool cFlag = Z & 0x1;
        Z                = Z >> 1;
        mem.write( readR16( Operand_t::hl ), Z );
        setZNHCFlags( ! Z, 0, 0, cFlag );
    } break;
    case SRL_R8: {
        const uint8_t value    = readR8( mop.operand1 );
        const bool cFlag       = value & 0x1;
        const uint8_t newValue = value >> 1;
        writeR8( mop.operand1, newValue );
        setZNHCFlags( ! newValue, 0, 0, cFlag );
    } break;
    case BIT_Z: {
        const bool bitSet = Z & ( 1 << std::to_underlying( mop.operand1 ) );
        setZNHCFlags( ! bitSet, 0, 1, getCFlag() );
    } break;
    case BIT_R8: {
        const uint8_t value = readR8( mop.operand2 );
        const bool bitSet   = value & ( 1 << std::to_underlying( mop.operand1 ) );
        setZNHCFlags( ! bitSet, 0, 1, getCFlag() );
    } break;
    case RES_pHL: {
        const uint16_t addr    = readR16( Operand_t::hl );
        const uint8_t value    = mem.read( addr );
        const uint8_t newValue = value & static_cast<uint8_t>( ~( 1 << std::to_underlying( mop.operand1 ) ) );
        mem.write( addr, newValue );
    } break;
    case RES_R8: {
        const uint8_t value    = readR8( mop.operand2 );
        const uint8_t newValue = value & static_cast<uint8_t>( ~( 1 << std::to_underlying( mop.operand1 ) ) );
        writeR8( mop.operand2, newValue );
    } break;
    case SET_pHL: {
        const uint16_t addr    = readR16( Operand_t::hl );
        const uint8_t value    = mem.read( addr );
        const uint8_t newValue = value | static_cast<uint8_t>( 1 << std::to_underlying( mop.operand1 ) );
        mem.write( addr, newValue );
    } break;
    case SET_R8: {
        const uint8_t value    = readR8( mop.operand2 );
        const uint8_t newValue = value | static_cast<uint8_t>( 1 << std::to_underlying( mop.operand1 ) );
        writeR8( mop.operand2, newValue );
    } break;
    case INVALID:
        break;
    case EMPTY:
        logFatal( ErrorCode::emptyMicroCodeExecuted, "Empty microcode executed! ( not NOP )" );
        logStacktrace();
        std::abort();
    default:
        logFatal( ErrorCode::InvalidOperandType,
                  std::format( "Unknown microcode executed, value: {0}", std::to_underlying( mop.type ) ) );
        logStacktrace();
        std::abort();
    }
}
