#pragma once
#undef abs
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>
using Enum_t = uint8_t;

class CPU {
    enum class OperationType_t : Enum_t {
        INVALID,
        NOOP,
        STOP,
        LD,
        INC,
        DEC,
        ADD,
        RLCA,
        RRCA,
        RLA,
        RRA,
        DAA,
        CPL,
        SCF,
        CCF,
        JR
    };
    enum class OperandType_t : Enum_t {
        NONE,
        R8,
        R16,
        R16STK,
        R16MEM,
        COND,
        B3_INDEX,
        TGT3,
        IMM8,
        IMM16
    };
    enum class Operand_t : Enum_t {
        // register enums also correspond to their place in registers array
        b,
        c,
        d,
        e,
        h,
        l,
        a,
        af = a,
        bc = b,
        de = d,
        hl = h,
        sp = 8,
        //the rest is not registers
        pHL, // byte pointed to by HL
        hlPlus,
        hlMinus,
        condNz,
        condZ,
        condNc,
        condC
    };

    //In case of IMM8 and IMM16, don't save the next byte(s)
    //They will be fetched in execution phase
    struct Operation_t {
        OperationType_t operationType;
        OperandType_t operandType1 = OperandType_t::NONE;
        std::variant<std::monostate, Operand_t, uint8_t> operand1;
        OperandType_t operandType2 = OperandType_t::NONE;
        std::variant<std::monostate, Operand_t, uint8_t> operand2;
    };

    uint8_t registers[10];
    uint16_t PC;
    uint8_t ROM[1024]; //placeholder

    uint16_t read16( Operand_t register_ );
    void write16( Operand_t register_, uint16_t value );
    Operation_t decode();
    //helpers
    Operation_t decodeBlock00();
    Operation_t decodeBlock01();
    Operation_t decodeBlock10();
    Operation_t decodeBlock11();

public:
};
