#include "cpu.hpp"

namespace {} // namespace

template<CPU::OperandType_t type, uint8or16_t T>
void CPU::write( OperandVar_t operand, T writeValue ) {
    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( opdVal );

    if constexpr( type == OperandType_t::R8 && std::same_as<T, uint8_t> ) {
        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            RAM[registers[index] | ( registers[index + 1] << 8 )] = writeValue;
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
            registers[aIndex] = writeValue;
        }
        registers[underlVal] = writeValue;
    }

    else if( std::same_as<T, uint16_t> ) {
        if constexpr( type == OperandType_t::R16 ) {
            if( opdVal == Operand_t::sp )
                SP = writeValue;
            else {
                registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
                registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
            }
        }

        else if constexpr( type == OperandType_t::R16STK ) {
            registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
            registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
        }

        else if constexpr( type == OperandType_t::R16MEM ) {
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
                static_assert( false, "" );
            }
        }

        else if constexpr( type == OperandType_t::pIMM16 ) {
            const uint16_t index = ROM[PC] | static_cast<uint16_t>( ( ROM[PC + 1] << 8 ) );
            PC += 2;
            RAM[index] = writeValue;
        }
    }

    else
        static_assert( false, "" );
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
            return registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 );
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
            static_assert( false, "" );
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
        static_assert( false, "" );
}

void CPU::execute( Operation_t op ) {
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
        switch( op.operandType1 ) {}
        break;
    }

    write<OperandType_t::R8>( { Operand_t::a }, uint8_t {} );
    read<OperandType_t::R8>( {} );
}
