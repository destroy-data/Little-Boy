#include "cpu.hpp"

namespace {} // namespace

template<CPU::OperandType_t type>
void CPU::write( OperandVar_t operand, std::unsigned_integral auto writeValue ) {
    constexpr int writeSize = sizeof( writeValue );
    //TODO TGT3 and B3 (?)

    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( std::get<Operand_t>( operand ) );
    if constexpr( type == OperandType_t::R8 && writeSize == 1 ) {
        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            RAM[registers[index] | ( registers[index + 1] << 8 )] = writeValue;
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
            registers[aIndex] = writeValue;
        }
    } else if constexpr( type == OperandType_t::R16 && writeSize == 2 ) {
        if( opdVal == Operand_t::sp )
            SP = writeValue;
        else {
            registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
            registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
        }
    } else if constexpr( type == OperandType_t::R16STK && writeSize == 2 ) {
        registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
        registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
    } else if constexpr( type == OperandType_t::R16MEM && writeSize == 2 ) {
        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            registers[underlVal * 2] = static_cast<uint8_t>( writeValue );
            registers[underlVal * 2 + 1] = static_cast<uint8_t>( writeValue >> 8 );
            break;
        case Operand_t::hlPlus:
            //TODO
            break;
        case Operand_t::hlMinus:
            //TODO
            break;
        default:
            static_assert( false, "" );
        }
    }

    //TODO IMM8, IMM16, pIMM16
}

template<CPU::OperandType_t type>
std::unsigned_integral auto CPU::read( const OperandVar_t operand ) {
    //TODO TGT3 and B3
    /*if constexpr( type == OperandType_t::TGT3 || type == OperandType_t::B3_INDEX ) {*/
    /*    const auto opdVal = std::get<uint8_t>( operand );*/
    /*}*/

    const auto opdVal = std::get<Operand_t>( operand );
    const auto underlVal = static_cast<Enum_t>( std::get<Operand_t>( operand ) );
    if constexpr( type == OperandType_t::R8 ) {
        if( opdVal == Operand_t::pHL ) {
            const auto index = static_cast<Enum_t>( Operand_t::hl ) * 2;
            return RAM[registers[index] | ( registers[index + 1] << 8 )];
        } else if( opdVal == Operand_t::a ) {
            const int aIndex = 6;
            return registers[aIndex];
        }
        return registers[underlVal];
    } else if constexpr( type == OperandType_t::R16 ) {
        if( opdVal == Operand_t::sp )
            return SP;
        else
            return registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 );
    } else if constexpr( type == OperandType_t::R16STK ) {
        return registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 );
    } else if constexpr( type == OperandType_t::R16MEM ) {
        const int hlIndex = 4;
        switch( opdVal ) {
        case Operand_t::bc:
        case Operand_t::de:
            return RAM[registers[underlVal * 2] | ( registers[underlVal * 2 + 1] << 8 )];
        case Operand_t::hlPlus:
            //TODO
            break;
        case Operand_t::hlMinus:
            //TODO
            break;
        default:
            static_assert( false, "" );
        }
    }

    else if constexpr( type == OperandType_t::IMM8 ) {
        return ROM[PC++];
    } else if constexpr( type == OperandType_t::IMM16 ) {
        const auto retVal = static_cast<uint16_t>( ROM[PC] | ( ROM[PC + 1] << 8 ) );
        PC += 2;
        return retVal;
    } else if constexpr( type == OperandType_t::pIMM16 ) {
        const auto index = static_cast<uint16_t>( ROM[PC] | ( ROM[PC + 1] << 8 ) );
        PC += 2;
        return RAM[index];
    } else {
        static_assert( false, "" );
    }
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
}
