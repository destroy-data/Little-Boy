#pragma once
#undef abs
#include <cstddef>
#include <cstdint>
#include <variant>
using Enum_t = uint8_t;

class CPU {
    enum class OperationType_t : Enum_t {
        INVALID,
        NOOP,
        STOP,
        HALT,
        LD,
        LDH,
        INC,
        DEC,
        ADD,
        ADC,
        SUB,
        SBC,
        AND,
        XOR,
        OR,
        CP,
        RLC,
        RRC,
        RL,
        RR,
        DAA,
        CPL,
        SCF,
        CCF,
        JR,
        JP,
        RET,
        RETI,
        CALL,
        RST,
        POP,
        PUSH,
        DI,
        EI,
        SLA,
        SRA,
        SWAP,
        SRL,
        BIT,
        RES,
        SET
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
        IMM16,
        pIMM8,
        pIMM16
    };
    enum class Operand_t : Enum_t {
        // values correspond to their encoding in opcodes
        // r8
        b = 0,
        c,
        d,
        e,
        h,
        l,
        pHL, //byte pointed to by HL
        a,
        // r16
        bc = 0,
        de,
        hl,
        sp,
        // r16stk
        af = 3,
        // r16mem
        hlPlus = 2,
        hlMinus,
        // cond
        condNz = 0,
        condZ,
        condNc,
        condC,
    };

    //In case of IMM8 and IMM16, don't save the next byte(s)
    //They will be fetched in execution phase
    struct Operation_t {
        OperationType_t operationType;
        OperandType_t operandType1;
        std::variant<std::monostate, Operand_t, uint8_t> operand1;
        OperandType_t operandType2;
        std::variant<std::monostate, Operand_t, uint8_t> operand2;
    };

    uint8_t registers[10];
    uint16_t PC;
    uint8_t ROM[1024]; //placeholder

    uint16_t read16( Operand_t register_ );
    void write16( Operand_t register_, uint16_t value );
    Operation_t decode();
    //helpers
    Operation_t decodeBlock0();
    Operation_t decodeBlock2();
    Operation_t decodeBlock3();
    Operation_t decodeCB();

public:
};
