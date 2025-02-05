#pragma once
#include <cstddef>
#include <cstdint>
#include <variant>

template<typename T>
concept uint8or16_t = std::same_as<T, uint8_t> || std::same_as<T, uint16_t>;

class CPU {
    using Enum_t = uint8_t;
    enum class OperationType_t : Enum_t {
        INVALID,
        NOP,
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
        RLCA,
        RRCA,
        RLA,
        RRA,
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
        RLC,
        RRC,
        RL,
        RR,
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
        pIMM16,
        SP_PLUS_IMM8
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
        condC
    };

    using OperandVar_t = std::variant<std::monostate, Operand_t, uint8_t>;
    //In case of IMM8 and IMM16, don't save the next byte(s)
    //They will be fetched in execution phase
    struct Operation_t {
        OperationType_t operationType;
        OperandType_t operandType1;
        OperandVar_t operand1;
        OperandType_t operandType2;
        OperandVar_t operand2;
    };

    uint8_t registers[8]; //b,c,d,e,h,l,a,f
    uint16_t SP, PC;
    uint8_t RAM[65536]; //placeholder
    uint8_t ROM[1024];  //placeholder

    uint16_t read16( Operand_t register_ );
    void write16( Operand_t register_, uint16_t value );
    template<OperandType_t type>
    uint8or16_t auto read( OperandVar_t operand );
    template<OperandType_t type>
    void write( OperandVar_t operand, uint8or16_t auto value );
    Operation_t decode();
    void execute( Operation_t op );
    //helpers
    Operation_t decodeBlock0();
    Operation_t decodeBlock2();
    Operation_t decodeBlock3();
    Operation_t decodeCB();

public:
};
