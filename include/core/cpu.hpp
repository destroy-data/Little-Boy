#pragma once
#include "core/logging.hpp"
#include "memory.hpp"
#include <array>
#include <cstdint>
#include <variant>

template<typename T>
concept uint8or16_t = std::same_as<T, uint8_t> || std::same_as<T, uint16_t>;

class CoreCpu {
protected:
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
        pR8,
        R16,
        R16STK,
        R16MEM,
        COND,
        BIT_INDEX,
        TGT3,
        IMM8,
        IMM16,
        pIMM16,
        SP_PLUS_IMM8,
        FF00_PLUS_R8
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
        condNZ = 0,
        condZ,
        condNC,
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
        Operation_t( OperationType_t operationType_, OperandType_t operandType1_ = {},
                     OperandVar_t operand1_ = {}, OperandType_t operandType2_ = {},
                     OperandVar_t operand2_ = {} )
            : operationType( operationType_ )
            , operandType1( operandType1_ )
            , operand1( operand1_ )
            , operandType2( operandType2_ )
            , operand2( operand2_ ) {
        }
    };

    uint8_t registers[8]; //b,c,d,e,h,l,a,f
    uint16_t SP, PC;
    Memory& mem;
    bool interruptMasterEnabled = false;

    static consteval size_t getOperandVarType( CoreCpu::OperandType_t operandType );
    template<OperandType_t type>
    uint8or16_t auto read( const OperandVar_t operand );
    template<OperandType_t type, uint8or16_t T>
    void write( OperandVar_t operand, T value );
    template<OperandType_t type, uint8or16_t T>
    void addTo( OperandVar_t operand, T value );
    template<OperandType_t type>
    void subFrom( OperandVar_t operand, uint8_t value, bool discard = false );
    template<OperationType_t optype>
    void bitwise( const Operation_t& op );
    template<OperationType_t optype>
    void bitShift( Operation_t op );

    void execute( const Operation_t& op );
    void handleInterrupts();
    void ld( const Operation_t& op );
    void ldh( const Operation_t& op );
    void pushToStack( uint16_t value );
    uint16_t popFromStack();
    bool isConditionMet( Operand_t condition );

    Operation_t decode();
    //helpers
    Operation_t decodeBlock0();
    Operation_t decodeBlock2();
    Operation_t decodeBlock3();
    Operation_t decodeCB();
    // clang-format off
    OperandType_t getR8Type( Operand_t operand ) { return operand == Operand_t::pHL ? OperandType_t::pR8 : OperandType_t::R8; }
    bool getZFlag() { return registers[7] &( 1 << 7 ); } // Zero flag
    bool getNFlag() { return registers[7] &( 1 << 6 ); } // BDC substraction flag
    bool getHFlag() { return registers[7] &( 1 << 5 ); } // BDC half carry flag
    bool getCFlag() { return registers[7] &( 1 << 4 ); } // Carry flag
    std::array<bool, 4> getZNHCFlags() { return { getZFlag(), getNFlag(), getHFlag(), getCFlag() }; }

    void setZFlag( bool val ) { registers[7] = static_cast<uint8_t>(val ? registers[7] | ( 1 << 7 ) : registers[7] & ~( 1 << 7 )); }
    void setNFlag( bool val ) { registers[7] = static_cast<uint8_t>(val ? registers[7] | ( 1 << 6 ) : registers[7] & ~( 1 << 6 )); }
    void setHFlag( bool val ) { registers[7] = static_cast<uint8_t>(val ? registers[7] | ( 1 << 5 ) : registers[7] & ~( 1 << 5 )); }
    void setCFlag( bool val ) { registers[7] = static_cast<uint8_t>(val ? registers[7] | ( 1 << 4 ) : registers[7] & ~( 1 << 4 )); }
    // clang-format on
    void setZNHCFlags( bool Z, bool N, bool H, bool C ) {
        setZFlag( Z );
        setNFlag( N );
        setHFlag( H );
        setCFlag( C );
    }

    virtual void handleJoypad() = 0;

public:
    CoreCpu( Memory& mem_ ) : mem( mem_ ) {
        logDebug( ErrorCode::NoError, "test" );
    };
    virtual ~CoreCpu() = default;
    void tick();
};
