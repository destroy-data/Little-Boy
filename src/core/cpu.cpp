#include "core/cpu.hpp"
#include "core/general_constants.hpp"
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
void CoreCpu::tick() {
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
}

bool CoreCpu::handleInterrupts() {
    if( ! interruptMasterEnabled )
        return false;
    const auto interruptEnable = mem.read( addr::interruptEnableRegister );
    const auto interruptFlag   = mem.read( addr::interruptFlag );
    bool executeInterrupt      = false;
    if( interruptEnable & 0x1 && interruptFlag & 0x1 ) { //VBlank
        setWZ( 0x40 );
        mem.write( addr::interruptFlag, interruptFlag & ~0x1 );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 1 ) && interruptFlag & ( 1 << 1 ) ) { //LCD
        setWZ( 0x48 );
        mem.write( addr::interruptFlag, interruptFlag & ~( 1 << 1 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 2 ) && interruptFlag & ( 1 << 2 ) ) { //Timer
        setWZ( 0x50 );
        mem.write( addr::interruptFlag, interruptFlag & ~( 1 << 2 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 3 ) && interruptFlag & ( 1 << 3 ) ) { //Serial
        setWZ( 0x58 );
        mem.write( addr::interruptFlag, interruptFlag & ~( 1 << 3 ) );
        executeInterrupt = true;
    } else if( interruptEnable & ( 1 << 4 ) && interruptFlag & ( 1 << 4 ) ) { //Joypad
        setWZ( 0x60 );
        mem.write( addr::interruptFlag, interruptFlag & ~( 1 << 4 ) );
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

void CoreCpu::logOperation( MicroOperation_t mop ) {
    std::string opd1;
    std::string opd2;
    // FIXME it's temporary solution
    logDebug( std::format( "MOT<{}>, opd1<{}>, opd2<{}>", MicroOperationTypeString[Enum_t( mop.type )],
                           Enum_t( mop.operand1 ), Enum_t( mop.operand2 ) ) );
    logDebug( std::format( "CPU flags ZNHC<{:04b}>", ( registers[7] >> 4 ) ) );
}

CoreCpu::CoreCpu( Memory& mem_ ) : mem( mem_ ) {
    //set register f
    const bool headerChecksumNonZero = mem.read( addr::headerChecksum );
    setZNHCFlags( 1, 0, headerChecksumNonZero, headerChecksumNonZero );
};
