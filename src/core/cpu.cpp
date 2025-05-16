#include "core/cpu.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <format>
#include <limits>
#include <utility>

#define invalidOperandType( OPERAND )                                                                         \
    do {                                                                                                      \
        logFatal( ErrorCode::InvalidOperandType, std::format( "Invalid operand " #OPERAND " with value: {0}", \
                                                              static_cast<Enum_t>( OPERAND ) ) );             \
        logStacktrace();                                                                                      \
        std::abort();                                                                                         \
    } while( false )


consteval size_t CoreCpu::getOperandVarType( CoreCpu::OperandType_t operandType ) {
    switch( operandType ) {
        using enum OperandType_t;
    case R8:
    case R16:
    case R16MEM:
    case R16STK:
    case pR8:
    case SP_PLUS_IMM8:
        return 1;
    case BIT_INDEX:
    case TGT3:
        return 2;
    default:
        return 0;
    }
}

template<CoreCpu::OperandType_t type, uint8or16_t T>
void CoreCpu::write( OperandVar_t operand, T writeValue ) {
    constexpr std::size_t writeSize = sizeof( T );
    const auto opdVal = std::get<getOperandVarType( type )>( operand );
    Enum_t underlVal;
    if constexpr( std::is_same_v<std::decay_t<decltype( opdVal )>, Operand_t> ) {
        underlVal = static_cast<Enum_t>( opdVal );
    };

    if constexpr( type == OperandType_t::R8 ) {
        static_assert( writeSize == 1, "R8 write: wrong value size" );

        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            mem.write( static_cast<uint16_t>( registers[index] | ( registers[index + 1] << 8 ) ),
                       static_cast<uint8_t>( writeValue ) );
        } else if( opdVal == Operand_t::a ) {
            constexpr int aIndex = 6;
            registers[aIndex] = writeValue;
        }
        registers[underlVal] = writeValue;
    }

    else if constexpr( type == OperandType_t::R16 ) {
        static_assert( writeSize == 2, "R16 write: wrong value size" );

        if( opdVal == Operand_t::sp )
            SP = writeValue;
        else {
            registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
            registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
        }
    }

    else if constexpr( type == OperandType_t::R16STK ) {
        static_assert( writeSize == 2, "R16STK write: wrong value size" );

        registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
        registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
    }

    else if constexpr( type == OperandType_t::R16MEM ) {
        static_assert( writeSize == 1, "R16MEM write: wrong value size" );

        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            mem.write(
                    static_cast<uint16_t>( registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 ) ),
                    writeValue );
            return;
        case Operand_t::hlPlus: {
            uint16_t currentHl = registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            mem.write( currentHl++, writeValue );
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl = registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            mem.write( currentHl--, writeValue );
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return;
        }
        default:
            logError( ErrorCode::InvalidOperand,
                      std::format( "Operand_t: {0}", static_cast<Enum_t>( opdVal ) ) );
            break;
        }
    }

    else if constexpr( type == OperandType_t::pIMM16 ) {
        static_assert( writeSize == 1, "pIMM16 write: wrong value size" );
        const uint16_t index = mem.read( PC ) | static_cast<uint16_t>( ( mem.read( PC + 1 ) << 8 ) );
        PC += 2;
        mem.write( index, static_cast<uint8_t>( writeValue ) );
    }

    else
        static_assert( false, "Wrong operand type" );
}

template<CoreCpu::OperandType_t type>
uint8or16_t auto CoreCpu::read( const OperandVar_t operand ) {
    const auto opdVal = std::get<getOperandVarType( type )>( operand );
    Enum_t underlVal;
    if constexpr( std::is_same_v<std::decay_t<decltype( opdVal )>, Operand_t> ) {
        underlVal = static_cast<Enum_t>( opdVal );
    };

    if constexpr( type == OperandType_t::R8 ) {
        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            return mem.read( static_cast<uint16_t>( registers[index] | ( registers[index + 1] << 8 ) ) );
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
            return registers[aIndex];
        }
        return registers[underlVal];
    }

    else if constexpr( type == OperandType_t::R16 ) {
        if( opdVal == Operand_t::sp )
            return SP;
        else
            return static_cast<uint16_t>( registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 ) );
    }

    else if constexpr( type == OperandType_t::R16STK ) {
        return static_cast<uint16_t>( registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 ) );
    }

    else if constexpr( type == OperandType_t::R16MEM ) {
        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            return mem.read( static_cast<uint16_t>( registers[underlVal * 2] |
                                                    ( registers[underlVal * 2 + 1] << 8 ) ) );
        case Operand_t::hlPlus: {
            uint16_t currentHl = registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = mem.read( currentHl++ );
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return retVal;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl = registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = mem.read( currentHl-- );
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return retVal;
        }
        default:
            logError( ErrorCode::InvalidOperand,
                      std::format( "Operand_t: {0}", static_cast<Enum_t>( opdVal ) ) );
            break;
        }
    }

    else if constexpr( type == OperandType_t::IMM8 ) {
        return mem.read( PC++ );
    }

    else if constexpr( type == OperandType_t::IMM16 ) {
        const auto retVal = static_cast<uint16_t>( mem.read( PC ) | ( mem.read( PC + 1 ) << 8 ) );
        PC += 2;
        return retVal;
    }

    else if constexpr( type == OperandType_t::pIMM16 ) {
        const auto index = static_cast<uint16_t>( mem.read( PC ) | ( mem.read( PC + 1 ) << 8 ) );
        PC += 2;
        return mem.read( index );
    }

    else
        static_assert( false, "Wrong operand type" );

    std::unreachable();
}

template<CoreCpu::OperandType_t type, uint8or16_t T>
void CoreCpu::addTo( OperandVar_t operand, T value ) {
    auto currentValue = read<type>( operand );
    bool cFlag = ( std::numeric_limits<T>::max() - currentValue < value );
    bool halfCarryFlag = std::same_as<decltype( currentValue ), uint8_t>
                                 ? ( ( currentValue & 0xF ) + ( value & 0xF ) ) > 0xF
                                 : ( ( currentValue & 0xFFF ) + ( value & 0xFFF ) ) > 0xFFF;

    currentValue += value;
    write<type>( operand, currentValue );
    setZNHCFlags( !currentValue, false, halfCarryFlag, cFlag );
};

template<CoreCpu::OperandType_t type>
void CoreCpu::subFrom( OperandVar_t operand, uint8_t value, bool discard ) {
    auto currentValue = read<type>( operand );
    bool cFlag = ( value > currentValue );
    //due to integer promotion substraction operands are promoted to ints
    bool halfCarryFlag = ( ( currentValue & 0xF ) - ( value & 0xF ) ) < 0;

    currentValue -= value;
    if( !discard )
        write<type>( operand, currentValue );
    setZNHCFlags( !currentValue, true, halfCarryFlag, cFlag );
};

bool CoreCpu::isConditionMet( Operand_t condition ) {
    using enum Operand_t;
    if( condition == condZ && getZFlag() )
        return true;
    else if( condition == condNZ && !getZFlag() )
        return true;
    else if( condition == condC && getCFlag() )
        return true;
    else if( condition == condNC && !getCFlag() )
        return true;

    return false;
}

//--------------------------------------------------
template<CoreCpu::OperationType_t optype>
void CoreCpu::bitwise( const Operation_t& op ) {
    static_assert( ( optype == OperationType_t::AND ) || ( optype == OperationType_t::OR ) ||
                           ( optype == OperationType_t::XOR ),
                   "" );

    auto a = read<OperandType_t::R8>( { Operand_t::a } );
    uint8_t readVal;
    if( op.operandType2 == OperandType_t::R8 )
        readVal = read<OperandType_t::R8>( op.operand2 );
    else if( op.operandType2 == OperandType_t::IMM8 ) {
        readVal = read<OperandType_t::IMM8>( op.operand2 );
    } else
        invalidOperandType( op.operandType2 );

    uint8_t result;
    if constexpr( optype == OperationType_t::AND )
        result = a & readVal;
    else if constexpr( optype == OperationType_t::OR )
        result = a | readVal;
    else if constexpr( optype == OperationType_t::XOR )
        result = a ^ readVal;
    else
        static_assert( false, "" );
    write<OperandType_t::R8>( { Operand_t::a }, result );

    constexpr bool hFlag = ( optype == OperationType_t::AND );
    setZNHCFlags( !result, false, hFlag, false );
}

template<CoreCpu::OperationType_t optype>
void CoreCpu::bitShift( Operation_t op ) {
    using OT = OperationType_t;

    if( std::holds_alternative<std::monostate>( op.operand1 ) ) {
        op.operand1 = { Operand_t::a };
    }
    auto value = read<OperandType_t::R8>( op.operand1 );

    bool zFlag, cFlag;
    int bits = ( getCFlag() << 9 ) | ( value << 1 ) | getCFlag();
    if constexpr( optype == OT::RL || optype == OT::RLA ) {
        cFlag = value & ( 1 << 7 );
        value = static_cast<uint8_t>( bits );
    } else if constexpr( optype == OT::RR || optype == OT::RRA ) {
        cFlag = value & 0x1;
        value = static_cast<uint8_t>( bits >> 2 );
    } else if constexpr( optype == OT::RLC || optype == OT::RLCA ) {
        cFlag = value & ( 1 << 7 );
        value = std::rotl( value, 1 );
    } else if constexpr( optype == OT::RRC || optype == OT::RRCA ) {
        cFlag = value & 0x1;
        value = std::rotr( value, 1 );
    } else if constexpr( optype == OT::SLA ) {
        cFlag = value & ( 1 << 7 );
        value <<= 1;
    } else if constexpr( optype == OT::SRA ) {
        cFlag = 0x1;
        value >>= 1;
        value |= ( value << 1 ) & ( 1 << 7 );
    } else if constexpr( optype == OT::SRL ) {
        cFlag = 0x1;
        value >>= 1;
    } else
        static_assert( false, "" );

    if constexpr( optype == OT::RLA || optype == OT::RRA || optype == OT::RLCA || optype == OT::RRCA )
        zFlag = false;
    else
        zFlag = !value;

    write<OperandType_t::R8>( op.operand1, value );
    setZNHCFlags( zFlag, false, false, cFlag );
}

void CoreCpu::pushToStack( uint16_t value ) {
    mem.write( --SP, static_cast<uint8_t>( ( value & 0xFF00 ) >> 8 ) );
    mem.write( --SP, static_cast<uint8_t>( value & 0xFF ) );
}

uint16_t CoreCpu::popFromStack() {
    uint16_t value = mem.read( SP++ );
    value |= static_cast<uint16_t>( mem.read( SP++ ) << 8 );
    return value;
}

void CoreCpu::ld( const Operation_t& op ) {
    uint16_t readVal; //also for 8-bit values, later truncate them if needed
    switch( op.operandType2 ) {
        using enum OperandType_t;
    case R8:
        readVal = read<R8>( op.operand2 );
        break;
    case R16:
        readVal = read<R16>( op.operand2 );
        break;
    case R16MEM:
        readVal = read<R16MEM>( op.operand2 );
        break;
    case IMM8:
        readVal = read<IMM8>( op.operand2 );
        break;
    case IMM16:
        readVal = read<IMM16>( op.operand2 );
        break;
    case pIMM16:
        readVal = read<pIMM16>( op.operand2 );
        break;
    case SP_PLUS_IMM8: {
        readVal = read<R16>( { Operand_t::sp } );
        const auto imm8 = read<IMM8>( op.operand2 );
        bool cFlag = ( std::numeric_limits<uint16_t>::max() - readVal < imm8 );
        bool halfCarryFlag = ( ( readVal & 0xFFF ) + ( imm8 & 0xFFF ) ) > 0xFFF;

        readVal += imm8;
        setZNHCFlags( !readVal, false, halfCarryFlag, cFlag );
    } break;
    default:
        invalidOperandType( op.operandType2 );
    }

    switch( op.operandType1 ) {
        using enum OperandType_t;
    case R8:
        write<R8>( op.operand1, static_cast<uint8_t>( readVal ) );
        break;
    case R16:
        write<R16>( op.operand1, readVal );
        break;
    case R16MEM:
        write<R16MEM>( op.operand1, static_cast<uint8_t>( readVal ) );
        break;
    case pIMM16:
        write<pIMM16>( op.operand1, static_cast<uint8_t>( readVal ) );
        break;
    default:
        invalidOperandType( op.operandType1 );
    }
}

void CoreCpu::ldh( const Operation_t& op ) {
    uint16_t readVal; //also for 8-bit values, later truncate them if needed
    switch( op.operandType2 ) {
        using enum OperandType_t;
    case R8:
        readVal = read<R8>( op.operand2 );
        break;
    case FF00_PLUS_R8:
        readVal = mem.read( 0xFF00 + read<R8>( op.operand2 ) );
        break;
    case IMM8:
        readVal = mem.read( 0xFF00 + read<IMM8>( op.operand2 ) );
        break;
    default:
        invalidOperandType( op.operandType2 );
    }

    switch( op.operandType1 ) {
        using enum OperandType_t;
    case R8:
        write<OperandType_t::R8>( op.operand1, static_cast<uint8_t>( readVal ) );
        break;
    case FF00_PLUS_R8:
        mem.write( 0xFF00 + read<R8>( op.operand1 ), static_cast<uint8_t>( readVal ) );
        break;
    case IMM8:
        mem.write( 0xFF00 + read<IMM8>( op.operand1 ), static_cast<uint8_t>( readVal ) );
        break;
    default:
        invalidOperandType( op.operandType1 );
    }
}

void CoreCpu::execute( const Operation_t& op ) {
    switch( op.operationType ) {
        using OT = OperationType_t;
        using opdt = OperandType_t;
        using opd = Operand_t;
    //Load instructions
    case OT::LD:
        ld( op );
        break;
    case OT::LDH:
        ldh( op );
        break;
    //Arithmetic instructions
    case OT::ADC: {
        uint8_t readVal = getCFlag();
        if( op.operandType2 == opdt::R8 )
            readVal += read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 ) {
            readVal += read<opdt::IMM8>( op.operand2 );
        } else
            invalidOperandType( op.operandType2 );

        CoreCpu::addTo<opdt::R8>( op.operand1, readVal );
    } break;
    case OT::ADD:
    case OT::SUB: {
        uint16_t readVal;
        switch( op.operandType2 ) {
            using enum opdt;
        case R16:
            readVal = read<R16>( op.operand2 );
            break;
        case R8:
            readVal = read<R8>( op.operand2 );
            break;
        case IMM8:
            readVal = read<IMM8>( op.operand2 );
            break;
        default:
            invalidOperandType( op.operandType2 );
        }

        bool _add { op.operationType == OT::ADD };
        if( op.operandType1 == opdt::R8 ) {
            // There is no write from R16 to R8
            if( _add )
                addTo<opdt::R8>( op.operand1, static_cast<uint8_t>( readVal ) );
            else
                subFrom<opdt::R8>( op.operand1, static_cast<uint8_t>( readVal ) );
        } else if( op.operandType1 == opdt::R16 ) {
            if( _add )
                addTo<opdt::R16>( op.operand1, readVal );
            else {
                //ERRTODO
            }
        }
    } break;
    case OT::CP: {
        uint8_t val2;
        if( op.operandType2 == opdt::R8 )
            val2 = read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 )
            val2 = read<opdt::IMM8>( op.operand2 );
        else
            return;
        subFrom<opdt::R8>( { opd::a }, val2, true );
    } break;
    case OT::DEC: {
        const auto cFlag = getCFlag();
        if( op.operandType1 == opdt::R8 )
            subFrom<opdt::R8>( op.operand1, 1 );
        else if( op.operandType1 == opdt::R16 ) {
            auto current = read<opdt::R16>( op.operand1 );
            write<opdt::R16>( op.operand1, --current );
        }
        setCFlag( cFlag );
    } break;
    case OT::INC: {
        const auto cFlag = getCFlag();
        if( op.operandType1 == opdt::R8 )
            addTo<opdt::R8>( op.operand1, static_cast<uint8_t>( 1 ) );
        else if( op.operandType1 == opdt::R16 ) {
            auto current = read<opdt::R16>( op.operand1 );
            write<opdt::R16>( op.operand1, ++current );
        }
        setCFlag( cFlag );
    } break;
    case OT::SBC: {
        uint8_t readVal = getCFlag();
        if( op.operandType2 == opdt::R8 )
            readVal += read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 ) {
            readVal += read<opdt::IMM8>( op.operand2 );
        } else {
            logError( ErrorCode::InvalidOperand,
                      std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
            return;
        }

        subFrom<opdt::R8>( op.operand1, readVal );
    } break;
    //Bitwise logic instructions
    case OT::AND:
        bitwise<OT::AND>( op );
        break;
    case OT::CPL:
        write<opdt::R8>( { Operand_t::a }, static_cast<uint8_t>( ~( read<opdt::R8>( { Operand_t::a } ) ) ) );
        setNFlag( true );
        setHFlag( true );
        break;
    case OT::OR:
        bitwise<OT::OR>( op );
        break;
    case OT::XOR:
        bitwise<OT::XOR>( op );
        break;
    //Bit flag instructions
    case OT::BIT: {
        auto bitIndex = std::get<uint8_t>( op.operand1 );
        auto value = read<opdt::R8>( op.operand2 );
        setZNHCFlags( !( value & ( 1 << bitIndex ) ), false, true, getCFlag() );
    } break;
    case OT::RES: {
        auto bitIndex = std::get<uint8_t>( op.operand1 );
        auto value = read<opdt::R8>( op.operand2 );
        value &= static_cast<uint8_t>( ~( 1 << bitIndex ) );
        write<opdt::R8>( op.operand2, value );
    } break;
    case OT::SET: {
        auto bitIndex = std::get<uint8_t>( op.operand1 );
        auto value = read<opdt::R8>( op.operand2 );
        value |= ( 1 << bitIndex );
        write<opdt::R8>( op.operand2, value );
    } break;
    //Bit shift instructions
    case OT::RL:
        bitShift<OT::RL>( op );
        break;
    case OT::RLA:
        bitShift<OT::RLA>( op );
        break;
    case OT::RLC:
        bitShift<OT::RLC>( op );
        break;
    case OT::RLCA:
        bitShift<OT::RLCA>( op );
        break;
    case OT::RR:
        bitShift<OT::RR>( op );
        break;
    case OT::RRA:
        bitShift<OT::RRA>( op );
        break;
    case OT::RRC:
        bitShift<OT::RRC>( op );
        break;
    case OT::RRCA:
        bitShift<OT::RRCA>( op );
        break;
    case OT::SLA:
        bitShift<OT::SLA>( op );
        break;
    case OT::SRA:
        bitShift<OT::SRA>( op );
        break;
    case OT::SRL:
        bitShift<OT::SRA>( op );
        break;
    case OT::SWAP: {
        auto value = read<opdt::R8>( op.operand1 );
        value = static_cast<uint8_t>( ( value >> 4 ) | ( value << 4 ) );
        write<opdt::R8>( op.operand1, value );
        setZNHCFlags( !value, false, false, false );
    } break;
    //Jumps and subroutine instructions
    case OT::CALL: {
        const bool withCondition = op.operandType1 == opdt::COND;
        if( withCondition ) {
            if( op.operandType2 != opdt::IMM16 ) {
                invalidOperandType( op.operandType2 );
            }
            const auto condition = std::get<Operand_t>( op.operand1 );
            if( !isConditionMet( condition ) )
                return;
        }
        pushToStack( PC + 3 );
        PC = read<opdt::IMM16>( withCondition ? op.operand2 : op.operand1 );
    } break;
    case OT::JP: {
        switch( op.operandType1 ) {
        case opdt::IMM16:
            PC = read<opdt::IMM16>( op.operand1 );
            break;
        case opdt::R16:
            PC = read<opdt::R16>( op.operand1 );
            break;
        case opdt::COND: {
            if( op.operandType2 != opdt::IMM16 )
                invalidOperandType( op.operandType2 );
            const auto condition = std::get<Operand_t>( op.operand1 );
            const auto address = read<opdt::IMM16>( op.operand2 );
            if( isConditionMet( condition ) )
                PC = address;
        } break;
        default:
            invalidOperandType( op.operandType1 );
        }
    } break;
    case OT::JR: {
        // When reading IMM8, PC is incremented by 1 and points to just read byte,
        // so when making jump relative to the byte immediately after this instruction
        // we need to add another 1 to PC before applying offset
        if( op.operandType1 == opdt::IMM8 ) {
            const auto offset = static_cast<int8_t>( read<opdt::IMM8>( op.operand1 ) );
            PC = static_cast<uint16_t>( PC + 1 + offset );
        } else if( op.operandType1 == opdt::COND && op.operandType2 == opdt::IMM8 ) {
            const auto condition = std::get<Operand_t>( op.operand1 );
            const auto offset = static_cast<int8_t>( read<opdt::IMM8>( op.operand2 ) );
            if( isConditionMet( condition ) )
                PC = static_cast<uint16_t>( PC + 1 + offset );
        } else
            invalidOperandType( op.operandType1 );

    } break;
    case OT::RET:
        if( op.operandType1 == opdt::NONE )
            PC = popFromStack();
        else if( op.operandType1 == opdt::COND ) {
            const auto condition = std::get<Operand_t>( op.operand1 );
            if( isConditionMet( condition ) )
                PC = popFromStack();
        } else {
            invalidOperandType( op.operandType1 );
        }
        break;
    case OT::RETI:
        interruptMasterEnabled = true; //TODO should be set right after this instruction
        PC = popFromStack();
        break;
    case OT::RST:
        if( op.operandType1 == opdt::TGT3 ) {
            const uint16_t jumpTo = std::get<uint8_t>( op.operand1 ) * 8;
            pushToStack( PC + 1 );
            PC = jumpTo;
        } else
            invalidOperandType( op.operandType1 );
        break;
    //Carry flag instructions
    case OT::CCF:
        setNFlag( false );
        setHFlag( false );
        setCFlag( !getCFlag() );
        break;
    case OT::SCF:
        setNFlag( false );
        setHFlag( false );
        setCFlag( true );
        break;
    //Stack manipulation instructions
    case OT::POP:
        write<opdt::R16STK>( op.operand1, popFromStack() );
        break;
    case OT::PUSH:
        pushToStack( read<opdt::R16STK>( op.operand1 ) );
        break;
    //Interrupt-related instructions
    case OT::DI:
        interruptMasterEnabled = false;
        break;
    case OT::EI:
        //This should be delayed by one instruction
        interruptMasterEnabled = true;
        break;
    case OT::HALT:
        //TODO
        break;
    //Miscellaneous instructions
    case OT::DAA: {
        uint8_t adjustment = 0;
        bool cFlag = false;
        if( getNFlag() ) {
            if( getHFlag() )
                adjustment += 6;
            if( getCFlag() )
                adjustment += 0x60;
            subFrom<opdt::R8>( { opd::a }, adjustment );
        } else {
            if( getHFlag() || ( read<opdt::R8>( { opd::a } ) & 0xF ) > 9 )
                adjustment += 6;
            if( getCFlag() || read<opdt::R8>( { opd::a } ) > 0x99 ) {
                adjustment += 0x60;
                cFlag = true;
            }
            addTo<opdt::R8>( { opd::a }, adjustment );
        }
        setZNHCFlags( !read<opdt::R8>( { opd::a } ), getNFlag(), false, cFlag );
    } break;
    case OT::NOP:
        break;
    case OT::STOP:
        //TODO
        break;
    //Invalid and unknown instructions
    case OT::INVALID:
    default:
        //ERRTODO
        break;
    }
}

void CoreCpu::handleInterrupts() {
    auto interruptEnable = mem.read( 0xFFFF );
    auto interruptFlag = mem.read( 0xFF0F );
    if( interruptEnable & 1 && interruptFlag & 1 ) { //VBlank
        pushToStack( PC );
        PC = 0x40;
    }
    if( interruptEnable & ( 1 << 1 ) && interruptFlag & ( 1 << 1 ) ) { //LCD
        pushToStack( PC );
        PC = 0x48;
    }
    if( interruptEnable & ( 1 << 2 ) && interruptFlag & ( 1 << 2 ) ) { //Timer
        pushToStack( PC );
        PC = 0x50;
    }
    if( interruptEnable & ( 1 << 3 ) && interruptFlag & ( 1 << 3 ) ) { //Serial
        pushToStack( PC );
        PC = 0x58;
    }
    if( interruptEnable & ( 1 << 4 ) && interruptFlag & ( 1 << 4 ) ) { //Joypad
        pushToStack( PC );
        PC = 0x60;
    }
};

void CoreCpu::tick() {
}
