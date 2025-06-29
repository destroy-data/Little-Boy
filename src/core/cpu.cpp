#include "core/cpu.hpp"
#include "core/core_constants.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <format>
#include <limits>
#include <utility>

// Decoding and execution have separate files

uint8_t CoreCpu::addU8ToU8( uint8_t value, uint8_t value2 ) {
    bool cFlag         = value + value2 > std::numeric_limits<uint8_t>::max();
    bool halfCarryFlag = ( ( value & 0xF ) + ( value2 & 0xF ) ) > 0xF;

    const uint8_t result = value + value2;
    setZNHCFlags( ! result, false, halfCarryFlag, cFlag );
    return result;
};

void CoreCpu::addToR8( Operand_t operand, uint8_t value ) {
    auto currentValue  = readR8( operand );
    bool cFlag         = currentValue + value > std::numeric_limits<uint8_t>::max();
    bool halfCarryFlag = ( ( currentValue & 0xF ) + ( value & 0xF ) ) > 0xF;

    currentValue += value;
    writeR8( operand, currentValue );
    setZNHCFlags( ! currentValue, false, halfCarryFlag, cFlag );
};

void CoreCpu::subFromR8( Operand_t operand, uint8_t value, bool discard ) {
    auto currentValue = readR8( operand );
    bool cFlag        = value > currentValue;
    //due to integer promotion substraction operands are promoted to ints
    bool halfCarryFlag = ( ( currentValue & 0xF ) - ( value & 0xF ) ) < 0;

    const uint8_t newValue = currentValue - value;
    if( ! discard )
        writeR8( operand, newValue );
    setZNHCFlags( ! newValue, true, halfCarryFlag, cFlag );
};

uint8_t CoreCpu::readR8( Operand_t opd ) {
    if( opd == Operand_t::a ) {
        const int aIndex = 6;
        return registers[aIndex];
    }
    return registers[std::to_underlying( opd )];
}

void CoreCpu::writeR8( Operand_t opd, uint8_t value ) {
    if( opd == Operand_t::a ) {
        constexpr int aIndex = 6;
        registers[aIndex]    = value;
    } else
        registers[std::to_underlying( opd )] = value;
}

uint16_t CoreCpu::readR16( Operand_t opd ) {
    if( opd == Operand_t::sp )
        return SP;
    else
        return static_cast<uint16_t>( registers[std::to_underlying( opd ) * 2] << 8 |
                                      ( registers[std::to_underlying( opd ) * 2 + 1] ) );
}

void CoreCpu::writeR16( Operand_t opd, uint16_t value ) {
    if( opd == Operand_t::sp )
        SP = value;
    else {
        registers[std::to_underlying( opd ) * 2]     = msb( value );
        registers[std::to_underlying( opd ) * 2 + 1] = lsb( value );
    }
}


bool CoreCpu::isConditionMet( Operand_t condition ) {
    using enum Operand_t;
    if( condition == condNZ && ! getZFlag() )
        return true;
    if( condition == condZ && getZFlag() )
        return true;
    if( condition == condNC && ! getCFlag() )
        return true;
    if( condition == condC && getCFlag() )
        return true;

    return false;
}

//--------------------------------------------------
unsigned CoreCpu::tick() {
    const MicroOperationType_t currentMopType = mopQueue[atMicroOperationNr].type;
    if( currentMopType == MicroOperationType_t::END && ! handleInterrupts() ) {
        logDebug( std::format( "PC: {} - Decoding next instruction", toHex( PC ) ) );
        mopQueue           = decode();
        atMicroOperationNr = 0;
    }

    execute( mopQueue[atMicroOperationNr] );
    if( ! lastConditionCheck && ( currentMopType == MicroOperationType_t::CHECK_COND ||
                                  currentMopType == MicroOperationType_t::COND_CHECK__LD_IMM_TO_W ||
                                  currentMopType == MicroOperationType_t::COND_CHECK__LD_IMM_TO_Z ) ) {
        mopQueue[atMicroOperationNr + 1] = { MicroOperationType_t::NOP };
        mopQueue[atMicroOperationNr + 2] = { MicroOperationType_t::END };
    }
    if( enableIMELater && atMicroOperationNr == 0 ) { // DI takes one cycle, so we are just after next one
        interruptMasterEnabled = true;
        enableIMELater         = false;
    }
    atMicroOperationNr++;
    return 4; // One M-cycle
}

bool CoreCpu::handleInterrupts() {
    if( ! interruptMasterEnabled )
        return false;
    const auto interruptEnable = bus.read( addr::interruptEnableRegister );
    const auto interruptFlag   = bus.read( addr::interruptFlag );
    bool executeInterrupt      = false;
    if( interruptEnable & 0x1 && interruptFlag & 0x1 ) { //VBlank
        setWZ( 0x40 );
        bus.write( addr::interruptFlag, interruptFlag & ~0x1 );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 1 ) && interruptFlag & ( 1 << 1 ) ) { //LCD
        setWZ( 0x48 );
        bus.write( addr::interruptFlag, interruptFlag & ~( 1 << 1 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 2 ) && interruptFlag & ( 1 << 2 ) ) { //Timer
        setWZ( 0x50 );
        bus.write( addr::interruptFlag, interruptFlag & ~( 1 << 2 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 3 ) && interruptFlag & ( 1 << 3 ) ) { //Serial
        setWZ( 0x58 );
        bus.write( addr::interruptFlag, interruptFlag & ~( 1 << 3 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 4 ) && interruptFlag & ( 1 << 4 ) ) { //Joypad
        setWZ( 0x60 );
        bus.write( addr::interruptFlag, interruptFlag & ~( 1 << 4 ) );
        executeInterrupt = true;
    }

    if( executeInterrupt ) {
        using enum MicroOperationType_t;
        mopQueue               = { { NOP, SP_DEC, LD_PCH_TO_SP, LD_PCL_TO_SP__LD_WZ_TO_PC, NOP } };
        atMicroOperationNr     = 0;
        interruptMasterEnabled = false;
        return true;
    }
    return false;
};


CoreCpu::CoreCpu( IBus& bus_ ) : bus( bus_ ) {
    //set register f
    const bool headerChecksumNonZero = bus.read( addr::headerChecksum );
    setZNHCFlags( 1, 0, headerChecksumNonZero, headerChecksumNonZero );
};


#ifdef DEBUG
constexpr std::string_view MicroOperationTypeString[] = {
        "NOP",
        "STOP",
        "LD_IMM_TO_Z",
        "LD_IMM_TO_W",
        "LD_SPL_TO_pWZ",
        "LD_SPH_TO_pWZ",
        "RLCA",
        "RRCA",
        "RLA",
        "RRA",
        "DAA",
        "CPL",
        "SCF",
        "CCF",
        "ALU_CALC_RELATIVE_JUMP",
        "IDU_LD_WZ_PLUS_1_TO_PC",
        "HALT",
        "ALU_ADD_Z_TO_A",
        "ALU_ADC_Z_TO_A",
        "ALU_SUB_Z_FROM_A",
        "ALU_SUB_Z_AND_C_FROM_A",
        "ALU_A_AND_Z",
        "ALU_A_XOR_Z",
        "ALU_A_OR_Z",
        "ALU_A_CP_Z",
        "ALU_ADD_SPL_TO_Z",
        "ALU_ADD_SPH_TO_W",
        "ALU_SPH_ADC_ADJ_TO_W",
        "LD_WZ_TO_SP",
        "POP_SP_TO_Z",
        "POP_SP_TO_W",
        "LD_WZ_TO_PC",
        "LD_WZ_TO_PC__ENABLE_IME",
        "JP_TO_HL",
        "SP_DEC",
        "LD_PCH_TO_SP",
        "LD_PCL_TO_SP__LD_WZ_TO_PC",
        "LD_A_TO_FF00_PLUS_C",
        "LD_A_TO_FF00_PLUS_Z",
        "LD_A_TO_pWZ",
        "LD_FF00_PLUS_C_TO_Z",
        "LD_Z_TO_R8",
        "LD_FF00_PLUS_Z_TO_Z",
        "LD_pWZ_TO_Z",
        "ALU_SPL_PLUS_Z_TO_L",
        "ALU_SPH_ADC_ADJ_TO_H",
        "LD_HL_TO_SP",
        "DI",
        "EI",
        "COND_CHECK__LD_IMM_TO_Z",
        "INC_R8",
        "LD_pHL_TO_Z",
        "ALU_LD_Z_PLUS_1_TO_pHL",
        "ALU_LD_Z_MINUS_1_TO_pHL",
        "DEC_R8",
        "LD_Z_TO_pHL",
        "LD_WZ_TO_R16",
        "LD_A_TO_R16_MEM",
        "IDU_INC_R16",
        "ALU_ADD_LSB_R16_TO_L",
        "ALU_ADC_MSB_R16_TO_H",
        "LD_R16_MEM_TO_Z",
        "IDU_DEC_R16",
        "LD_R8_TO_R8",
        "LD_R8_TO_pHL",
        "ALU_ADD_R8_TO_A",
        "ALU_ADC_R8_TO_A",
        "ALU_SUB_R8_FROM_A",
        "ALU_SBC_Z_FROM_A",
        "ALU_SBC_R8_FROM_A",
        "ALU_A_AND_R8",
        "ALU_A_XOR_R8",
        "ALU_A_OR_R8",
        "ALU_CP_A_R8",
        "CHECK_COND",
        "COND_CHECK__LD_IMM_TO_W",
        "LD_PCL_TO_SP__LD_TGT3_TO_PC",
        "LD_WZ_TO_R16STK",
        "PUSH_MSB_R16STK_TO_SP",
        "PUSH_LSB_R16STK_TO_SP",
        "FETCH_SECOND_BYTE",
        "LD_RLC_Z_TO_pHL",
        "RLC_R8",
        "LD_RRC_Z_TO_pHL",
        "RRC_R8",
        "LD_RL_Z_TO_pHL",
        "RL_R8",
        "LD_RR_Z_TO_pHL",
        "RR_R8",
        "LD_SLA_Z_TO_pHL",
        "SLA_R8",
        "LD_SRA_Z_TO_pHL",
        "SRA_R8",
        "LD_SWAP_Z_TO_pHL",
        "SWAP_R8",
        "LD_SRL_Z_TO_pHL",
        "SRL_R8",
        "BIT_Z",
        "BIT_R8",
        "RES_pHL",
        "RES_R8",
        "SET_pHL",
        "SET_R8",
        "INVALID",
        "END",
};
#endif


void CoreCpu::logOperation( [[maybe_unused]] MicroOperation_t mop ) {
#ifdef DEBUG
    std::string opd1;
    std::string opd2;
    // FIXME it's temporary solution
    logDebug( std::format( "MOT<{}>, opd1<{}>, opd2<{}>", MicroOperationTypeString[Enum_t( mop.type )],
                           Enum_t( mop.operand1 ), Enum_t( mop.operand2 ) ) );
    logDebug( std::format( "CPU flags ZNHC<{:04b}>", ( registers[7] >> 4 ) ) );
#endif
}
