#include "core/cpu.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <format>
#include <limits>
#include <utility>

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::write( OperandVar_t operand, T writeValue ) {
    constexpr std::size_t writeSize = sizeof( T );
    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( opdVal );

    if constexpr( type == OperandType_t::R8 ) {
        static_assert( writeSize == 1, "R8 write: wrong value size" );

        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            mem[registers[index] | ( registers[index + 1] << 8 )] =
                    static_cast<uint8_t>( writeValue );
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
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
            mem[registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 )] = writeValue;
            return;
        case Operand_t::hlPlus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            mem[currentHl++] = writeValue;
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            mem[currentHl--] = writeValue;
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
        const uint16_t index = mem[PC] | static_cast<uint16_t>( ( mem[PC + 1] << 8 ) );
        PC += 2;
        mem[index] = static_cast<uint8_t>( writeValue );
    }

    else
        static_assert( false, "Wrong operand type" );
}

template<CPU::OperandType_t type>
uint8or16_t auto CPU::read( const OperandVar_t operand ) {
    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( opdVal );

    if constexpr( type == OperandType_t::R8 ) {
        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            return mem[registers[index] | ( registers[index + 1] << 8 )];
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
            return static_cast<uint16_t>( registers[underlVal * 2] |
                                          ( registers[underlVal * 2 + 1] << 8 ) );
    }

    else if constexpr( type == OperandType_t::R16STK ) {
        return static_cast<uint16_t>( registers[underlVal * 2] |
                                      ( registers[underlVal * 2 + 1] << 8 ) );
    }

    else if constexpr( type == OperandType_t::R16MEM ) {
        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            return mem[registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 )];
        case Operand_t::hlPlus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = mem[currentHl++];
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return retVal;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = mem[currentHl--];
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
        return mem[PC++];
    }

    else if constexpr( type == OperandType_t::IMM16 ) {
        const auto retVal = static_cast<uint16_t>( mem[PC] | ( mem[PC + 1] << 8 ) );
        PC += 2;
        return retVal;
    }

    else if constexpr( type == OperandType_t::pIMM16 ) {
        const auto index = static_cast<uint16_t>( mem[PC] | ( mem[PC + 1] << 8 ) );
        PC += 2;
        return mem[index];
    }

    else
        static_assert( false, "Wrong operand type" );

    std::unreachable();
}

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::addTo( OperandVar_t operand, T value ) {
    auto currentValue = read<type>( operand );
    bool cFlag = ( std::numeric_limits<T>::max() - currentValue < value );
    bool halfCarryFlag = std::same_as<decltype( currentValue ), uint8_t>
                                 ? ( ( currentValue & 0xF ) + ( value & 0xF ) ) > 0xF
                                 : ( ( currentValue & 0xFFF ) + ( value & 0xFFF ) ) > 0xFFF;

    currentValue += value;
    write<type>( operand, currentValue );
    setZNHCFlags( !currentValue, false, halfCarryFlag, cFlag );
};

template<CPU::OperandType_t type>
void CPU::subFrom( OperandVar_t operand, uint8_t value, bool discard ) {
    auto currentValue = read<type>( operand );
    bool cFlag = ( value > currentValue );
    //due to integer promotion substraction operands are promoted to ints
    bool halfCarryFlag = ( ( currentValue & 0xF ) - ( value & 0xF ) ) < 0;

    currentValue -= value;
    if( !discard )
        write<type>( operand, currentValue );
    setZNHCFlags( !currentValue, true, halfCarryFlag, cFlag );
};

template<CPU::OperationType_t optype>
void CPU::bitwise( const Operation_t& op ) {
    static_assert( ( optype == OperationType_t::AND ) || ( optype == OperationType_t::OR ) ||
                           ( optype == OperationType_t::XOR ),
                   "" );

    auto a = read<OperandType_t::R8>( { Operand_t::a } );
    uint8_t readVal;
    if( op.operandType2 == OperandType_t::R8 )
        readVal = read<OperandType_t::R8>( op.operand2 );
    else if( op.operandType2 == OperandType_t::IMM8 ) {
        readVal = read<OperandType_t::IMM8>( op.operand2 );
    } else {
        logError( ErrorCode::InvalidOperand,
                  std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
        return;
    }
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

template<CPU::OperationType_t optype>
void CPU::bitShift( Operation_t op ) {
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

    if constexpr( optype == OT::RLA || optype == OT::RRA || optype == OT::RLCA ||
                  optype == OT::RRCA )
        zFlag = false;
    else
        zFlag = !value;

    write<OperandType_t::R8>( op.operand1, value );
    setZNHCFlags( zFlag, false, false, cFlag );
}

void CPU::pushToStack( uint16_t value ) {
    mem[--SP] = static_cast<uint8_t>( ( value & 0xFF00 ) >> 8 );
    mem[--SP] = static_cast<uint8_t>( value & 0xFF );
}

uint16_t CPU::popFromStack() {
    uint16_t value = mem[SP++];
    value |= static_cast<uint16_t>( mem[SP++] << 8 );
    return value;
}

void CPU::ld( const Operation_t& op ) {
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
    case SP_PLUS_IMM8:
        readVal = read<R16>( { Operand_t::sp } );
        readVal += read<IMM8>( op.operand2 );
        break;
    default:
        logError( ErrorCode::InvalidOperand,
                  std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
        return;
    }

    auto readVal8 = static_cast<uint8_t>( readVal );
    switch( op.operandType1 ) {
        using enum OperandType_t;
    case R8:
        write<R8>( op.operand1, readVal8 );
        break;
    case R16:
        write<R16>( op.operand1, readVal );
        break;
    case R16MEM:
        write<R16MEM>( op.operand1, readVal8 );
        break;
    case pIMM16:
        write<pIMM16>( op.operand1, readVal8 );
        break;
    default:
        logError( ErrorCode::InvalidOperand,
                  std::format( "op.operandType1: {0}", static_cast<Enum_t>( op.operandType1 ) ) );
        return;
    }
}

void CPU::ldh( const Operation_t& op ) {
    uint16_t readVal; //also for 8-bit values, later truncate them if needed
    switch( op.operandType2 ) {
        using enum OperandType_t;
    case R8:
        readVal = read<OperandType_t::R8>( op.operand2 );
        break;
    case IMM8:
        readVal = 0xFF00 + read<OperandType_t::IMM8>( op.operand2 );
        break;
    case pIMM16:
        readVal = read<OperandType_t::pIMM16>( op.operand2 );
        if( readVal < 0xFF00 )
            return;
        break;
    default:
        logError( ErrorCode::InvalidOperand,
                  std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
        return;
    }

    auto readVal8 = static_cast<uint8_t>( readVal );
    switch( op.operandType1 ) {
        using enum OperandType_t;
    case R8:
        write<OperandType_t::R8>( op.operand1, readVal8 );
        break;
    case R16:
        write<OperandType_t::R16>( op.operand1, readVal );
        break;
    case pIMM16:
        write<OperandType_t::pIMM16>( op.operand1, readVal8 );
        break;
    default:
        logError( ErrorCode::InvalidOperand,
                  std::format( "op.operandType1: {0}", static_cast<Enum_t>( op.operandType1 ) ) );
        return;
    }
}

void CPU::execute( const Operation_t& op ) {
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
        } else {
            logError(
                    ErrorCode::InvalidOperand,
                    std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
            return;
        }

        CPU::addTo<opdt::R8>( op.operand1, readVal );
    } break;
    case OT::ADD:
    case OT::SUB: {
        uint16_t readVal;
        switch( op.operandType2 ) {
            using enum opdt;
        case R16:
            readVal = read<opdt::R16>( op.operand2 );
            break;
        case R8:
            readVal = read<opdt::R8>( op.operand2 );
            break;
        case IMM8:
            readVal = read<opdt::IMM8>( op.operand2 );
            break;
        default:
            logError(
                    ErrorCode::InvalidOperand,
                    std::format( "op.operandType2: {0}", static_cast<Enum_t>( op.operandType2 ) ) );
            return;
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
    case OT::DEC:
        if( op.operandType1 == opdt::R8 ) {
            auto current = read<opdt::R8>( op.operand1 );
            write<opdt::R8>( op.operand1, --current );
        } else if( op.operandType1 == opdt::R16 ) {
            auto current = read<opdt::R16>( op.operand1 );
            write<opdt::R16>( op.operand1, --current );
        }
        break;
    case OT::INC:
        if( op.operandType1 == opdt::R8 ) {
            auto current = read<opdt::R8>( op.operand1 );
            write<opdt::R8>( op.operand1, ++current );
        } else if( op.operandType1 == opdt::R16 ) {
            auto current = read<opdt::R16>( op.operand1 );
            write<opdt::R16>( op.operand1, ++current );
        }
        break;
    case OT::SBC: {
        uint8_t readVal = getCFlag();
        if( op.operandType2 == opdt::R8 )
            readVal += read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 ) {
            readVal += read<opdt::IMM8>( op.operand2 );
        } else {
            logError(
                    ErrorCode::InvalidOperand,
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
        write<opdt::R8>( { Operand_t::a },
                         static_cast<uint8_t>( ~( read<opdt::R8>( { Operand_t::a } ) ) ) );
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
        setZNHCFlags( value & ( 1 << bitIndex ), false, true, getCFlag() );
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
        pushToStack( PC + 3 );
        PC = read<opdt::IMM16>( op.operand1 );
    } break;
    case OT::JP: {
        if( op.operandType1 == opdt::IMM16 )
            PC = read<opdt::IMM16>( op.operand1 );
        else if( op.operandType1 == opdt::R16 )
            PC = read<opdt::R16>( op.operand1 );
        else if( op.operandType1 == opdt::COND && op.operandType2 == opdt::IMM16 ) {
            auto condition = std::get<Operand_t>( op.operand1 );
            auto address = read<opdt::IMM16>( op.operand2 );
            if( condition == opd::condZ && getZFlag() )
                PC = address;
            else if( condition == opd::condNZ && !getZFlag() )
                PC = address;
            else if( condition == opd::condC && getCFlag() )
                PC = address;
            else if( condition == opd::condNC && !getCFlag() )
                PC = address;
        } else {
            logError(
                    ErrorCode::InvalidOperand,
                    std::format( "op.operandType1: {0}", static_cast<Enum_t>( op.operandType1 ) ) );
            return;
        }
    } break;
    case OT::JR: {
        // When reading IMM8, PC is incremented by 1 and points to just read byte,
        // so when making jump relative to the byte immediately after this instruction
        // we need to add another 1 to PC before applying offset
        if( op.operandType1 == opdt::IMM8 )
            PC = static_cast<uint16_t>( PC + 1 +
                                        static_cast<int8_t>( read<opdt::IMM8>( op.operand1 ) ) );
        else if( op.operandType1 == opdt::COND && op.operandType2 == opdt::IMM8 ) {
            auto condition = std::get<Operand_t>( op.operand1 );
            auto offset = static_cast<int8_t>( read<opdt::IMM8>( op.operand2 ) );
            if( condition == opd::condZ && getZFlag() )
                PC = static_cast<uint16_t>( PC + 1 + offset );
            else if( condition == opd::condNZ && !getZFlag() )
                PC = static_cast<uint16_t>( PC + 1 + offset );
            else if( condition == opd::condC && getCFlag() )
                PC = static_cast<uint16_t>( PC + 1 + offset );
            else if( condition == opd::condNC && !getCFlag() )
                PC = static_cast<uint16_t>( PC + 1 + offset );
        } else {
            logError(
                    ErrorCode::InvalidOperand,
                    std::format( "op.operandType1: {0}", static_cast<Enum_t>( op.operandType1 ) ) );
            return;
        }
    } break;
    case OT::RET:
        if( op.operandType1 == opdt::NONE )
            PC = popFromStack();
        else if( op.operandType1 == opdt::COND ) {
            auto condition = std::get<Operand_t>( op.operand1 );
            if( condition == opd::condZ && getZFlag() )
                PC = popFromStack();
            else if( condition == opd::condNZ && !getZFlag() )
                PC = popFromStack();
            else if( condition == opd::condC && getCFlag() )
                PC = popFromStack();
            else if( condition == opd::condNC && !getCFlag() )
                PC = popFromStack();
        } else {
            logError(
                    ErrorCode::InvalidOperand,
                    std::format( "op.operandType1: {0}", static_cast<Enum_t>( op.operandType1 ) ) );
            return;
        }
        break;
    case OT::RETI:
        interruptMasterEnabled = true;
        PC = popFromStack();
        break;
    case OT::RST:
        //TODO
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

void CPU::handleInterrupts() {
    auto interruptEnable = mem( 0xFFFF );
    auto interruptFlag = mem( 0xFF0F );
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
