#include "cpu.hpp"
#include <cstdint>
#include <exception>

namespace {} // namespace

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::write( OperandVar_t operand, T writeValue ) {
    constexpr size_t writeSize = sizeof( T );
    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( opdVal );

    if constexpr( type == OperandType_t::R8 ) {
        if constexpr( writeSize != 1 )
            static_assert( false, "R8 write: wrong value size" );

        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            RAM[registers[index] | ( registers[index + 1] << 8 )] =
                    static_cast<uint8_t>( writeValue );
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
            registers[aIndex] = static_cast<uint8_t>( writeValue );
        }
        registers[underlVal] = static_cast<uint8_t>( writeValue );
    }

    else if constexpr( type == OperandType_t::R16 ) {
        if constexpr( writeSize != 2 )
            static_assert( false, "R16 write: wrong value size" );

        if( opdVal == Operand_t::sp )
            SP = writeValue;
        else {
            registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
            registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
        }
    }

    else if constexpr( type == OperandType_t::R16STK ) {
        if constexpr( writeSize != 2 )
            static_assert( false, "R16STK write: wrong value size" );

        registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
        registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
    }

    else if constexpr( type == OperandType_t::R16MEM ) {
        if constexpr( writeSize != 1 )
            static_assert( false, "R16MEM write: wrong value size" );

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
        if constexpr( writeSize != 1 )
            static_assert( false, "pIMM16 write: wrong value size" );
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

void CPU::ld( const Operation_t& op ) {
    uint16_t readVal; //also for 8-bit values, later truncate them if needed
    switch( op.operandType2 ) {
        using enum OperandType_t;
    case R8:
        readVal = read<OperandType_t::R8>( op.operand2 );
        break;
    case R16:
        readVal = read<OperandType_t::R16>( op.operand2 );
        break;
    case R16MEM:
        readVal = read<OperandType_t::R16MEM>( op.operand2 );
        break;
    case IMM8:
        readVal = read<OperandType_t::IMM8>( op.operand2 );
        break;
    case IMM16:
        readVal = read<OperandType_t::IMM16>( op.operand2 );
        break;
    case pIMM16:
        readVal = read<OperandType_t::pIMM16>( op.operand2 );
        break;
    default:
        //ERRTODO
        std::terminate();
        break;
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
    case R16MEM:
        write<OperandType_t::R16MEM>( op.operand1, readVal8 );
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
    case OT::LD:
        ld( op );
        break;
    }

    write<OperandType_t::R8>( { Operand_t::a }, uint8_t {} );
    read<OperandType_t::R8>( {} );
}
