#include "cpu.hpp"
#include <cstdint>
#include <utility>

namespace {} // namespace

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::write( OperandVar_t operand, T writeValue ) {
    constexpr size_t writeSize = sizeof( T );
    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( opdVal );

    if constexpr( type == OperandType_t::R8 ) {
        static_assert( writeSize == 1, "R8 write: wrong value size" );

        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            RAM[registers[index] | ( registers[index + 1] << 8 )] =
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
            RAM[registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 )] = writeValue;
            return;
        case Operand_t::hlPlus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            RAM[currentHl++] = writeValue;
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            RAM[currentHl--] = writeValue;
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return;
        }
        default:
            //ERRTODO
            break;
        }
    }

    else if constexpr( type == OperandType_t::pIMM16 ) {
        static_assert( writeSize == 1, "pIMM16 write: wrong value size" );
        const uint16_t index = ROM[PC] | static_cast<uint16_t>( ( ROM[PC + 1] << 8 ) );
        PC += 2;
        RAM[index] = static_cast<uint8_t>( writeValue );
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
            return RAM[registers[index] | ( registers[index + 1] << 8 )];
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
        return registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 );
    }

    else if constexpr( type == OperandType_t::R16MEM ) {
        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            return RAM[registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 )];
        case Operand_t::hlPlus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = RAM[currentHl++];
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return retVal;
        }
        case Operand_t::hlMinus: {
            uint16_t currentHl =
                    registers[hlIndex] | static_cast<uint16_t>( registers[hlIndex + 1] << 8 );
            const auto retVal = RAM[currentHl--];
            write<OperandType_t::R16>( Operand_t::hl, currentHl );
            return retVal;
        }
        default:
            //ERRTODO
            break;
        }
    }

    else if constexpr( type == OperandType_t::IMM8 ) {
        return ROM[PC++];
    }

    else if constexpr( type == OperandType_t::IMM16 ) {
        const auto retVal = static_cast<uint16_t>( ROM[PC] | ( ROM[PC + 1] << 8 ) );
        PC += 2;
        return retVal;
    }

    else if constexpr( type == OperandType_t::pIMM16 ) {
        const auto index = static_cast<uint16_t>( ROM[PC] | ( ROM[PC + 1] << 8 ) );
        PC += 2;
        return RAM[index];
    }

    else
        static_assert( false, "Wrong operand type" );
}

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::addTo( OperandVar_t operand, T value ) {
    auto currentValue = read<type>( operand );
    currentValue += value;
    write<type>( operand, value );
};

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::subFrom( OperandVar_t operand, T value ) {
    auto currentValue = read<type>( operand );
    currentValue -= value;
    write<type>( operand, value );
};

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
        //ERRTODO
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
        //ERRTODO
        break;
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
        //ERRTODO
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
        //ERRTODO
        break;
    }
}

void CPU::execute( const Operation_t& op ) {
    switch( op.operationType ) {
        using OT = OperationType_t;
        using opdt = OperandType_t;
        using opd = Operand_t;
    case OT::LD:
        ld( op );
        break;
    case OT::LDH:
        ldh( op );
        break;
    case OT::ADC: {
        uint8_t readVal = getCFlag();
        if( op.operandType2 == opdt::R8 )
            readVal += read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 ) {
            readVal += read<opdt::IMM8>( op.operand2 );
        } else {
            //ERRTODO
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
            //ERRTODO
            return;
        }

        bool _add { op.operationType == OT::ADD };
        if( op.operandType1 == opdt::R8 )
            // There is no write from R16 to R8
            if( _add )
                addTo<opdt::R8>( op.operand1, static_cast<uint8_t>( readVal ) );
            else
                subFrom<opdt::R8>( op.operand1, static_cast<uint8_t>( readVal ) );
        else if( op.operandType1 == opdt::R16 ) {
            if( _add )
                addTo<opdt::R16>( op.operand1, readVal );
            else
                subFrom<opdt::R16>( op.operand1, readVal );
        }
    } break;
    case OT::CP: {
        auto val1 = read<opdt::R8>( opd::a );
        uint8_t val2;
        if( op.operandType2 == opdt::R8 )
            val2 = read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 )
            val2 = read<opdt::IMM8>( op.operand2 );
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
            readVal -= read<opdt::R8>( op.operand2 );
        else if( op.operandType2 == opdt::IMM8 ) {
            readVal -= read<opdt::IMM8>( op.operand2 );
        } else {
            //ERRTODO
        }

        CPU::addTo<opdt::R8>( op.operand1, readVal );
    } break;
    case OT::CPL:
        write<opdt::R8>( { Operand_t::a },
                         static_cast<uint8_t>( ~( read<opdt::R8>( { Operand_t::a } ) ) ) );
        break;
    case OT::INVALID:
        //TODO
        break;
    case OT::NOP:
        //TODO
        break;
    case OT::STOP:
        //TODO
        break;
    case OT::HALT:
        //TODO
        break;
    }

    write<OperandType_t::R8>( { Operand_t::a }, uint8_t {} );
    read<OperandType_t::R8>( {} );
}
