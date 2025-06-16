#pragma once
#include "core/cycles.hpp"
#include "memory.hpp"
#include <array>
#include <cstdint>
#include <variant>

template<typename T>
concept uint8or16_t = std::same_as<T, uint8_t> || std::same_as<T, uint16_t>;

class CoreCpu {
public:
    using Enum_t = uint8_t;
    enum class MicroOperationType_t : Enum_t {
        NOP,
        STOP,
        LD_IMM_TO_Z,
        LD_IMM_TO_W,
        LD_SPL_TO_pWZ,
        LD_SPH_TO_pWZ,
        RLCA,
        RRCA,
        RLA,
        RRA,
        DAA,
        CPL,
        SCF,
        CCF,
        ALU_CALC_RELATIVE_JUMP,
        IDU_LD_WZ_PLUS_1_TO_PC,
        HALT,
        ALU_ADD_Z_TO_A,
        ALU_ADD_Z_AND_C_TO_A,
        ALU_SUB_Z_FROM_A,
        ALU_SUB_Z_AND_C_FROM_A,
        ALU_AND_AZ,
        ALU_XOR_AZ,
        ALU_OR_AZ,
        ALU_CP_AZ,
        ALU_ADD_SPL_TO_Z,
        ALU_ADD_SPH_TO_W,
        ALU_SPH_PLUS_CADJ_TO_W,
        LD_WZ_TO_SP,
        LD_SPL_TO_Z,
        LD_SPH_TO_W,
        LD_WZ_TO_PC,
        LD_WZ_TO_PC_AND_ENABLE_IME,
        JP_TO_pHL,
        SP_DEC,
        LD_PCH_TO_SP,
        LD_PCL_TO_SP,
        LD_A_TO_FF00_PLUS_C,
        LD_A_TO_FF00_PLUS_Z,
        LD_A_TO_WZ,
        LD_FF00_PLUS_C_TO_Z,
        LD_Z_TO_R8,
        LD_FF00_PLUS_Z_TO_Z,
        LD_WZ_TO_Z,
        ALU_SPL_PLUS_Z_TO_L,
        ALU_SPH_PLUS_CADJ_TO_H,
        LD_HL_TO_SP,
        DI,
        EI,
        COND_CHECK_IMM8e,
        INC_R8,
        LD_pHL_TO_Z,
        ALU_LD_Z_PLUS_1_TO_pHL,
        ALU_LD_Z_MINUS_1_TO_pHL,
        DEC_R8,
        LD_Z_TO_pHL,
        LD_WZ_TO_R16,
        LD_R16_MEM_TO_A,
        IDU_INC_R16,
        ALU_ADD_LSB_R16_TO_L,
        ALU_ADD_CMSB_R16_TO_H,
        LD_R16_MEM_TO_Z,
        IDU_DEC_R16,


        INVALID,
        EMPTY
    };
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
    static constexpr std::string_view OperationTypeString[] = {
            "INVALID", "NOP", "STOP", "HALT", "LD",   "LDH",  "INC", "DEC",  "ADD", "ADC", "SUB", "SBC",
            "AND",     "XOR", "OR",   "CP",   "RLCA", "RRCA", "RLA", "RRA",  "DAA", "CPL", "SCF", "CCF",
            "JR",      "JP",  "RET",  "RETI", "CALL", "RST",  "POP", "PUSH", "DI",  "EI",  "RLC", "RRC",
            "RL",      "RR",  "SLA",  "SRA",  "SWAP", "SRL",  "BIT", "RES",  "SET",
    };

    enum class OperandType_t : Enum_t {
        NONE,
        R8,
        pHL,
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
    static constexpr std::string_view OperandTypeString[] = {
            "NONE",      "R8",   "pHL",  "R16",   "R16STK", "R16MEM",       "COND",
            "BIT_INDEX", "TGT3", "IMM8", "IMM16", "pIMM16", "SP_PLUS_IMM8", "FF00_PLUS_R8",
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
        phl,
        a,
        // r16
        bc = 0,
        de,
        hl,
        sp,
        // r16stk
        af = 3,
        // r16mem
        pBC = 0,
        pDE,
        hlPlus,
        hlMinus,
        // cond
        condNZ = 0,
        condZ,
        condNC,
        condC,
        NONE
    };

    using OperandVar_t = std::variant<std::monostate, Operand_t, uint8_t>;
    //In case of IMM8 and IMM16, don't save the next byte(s)
    //They will be fetched in execution phase
    struct Operation_t {
        uint16_t opcode;
        OperationType_t operationType;
        OperandType_t operandType1;
        OperandVar_t operand1;
        OperandType_t operandType2;
        OperandVar_t operand2;
        Operation_t( uint16_t opcode_, OperationType_t operationType_, OperandType_t operandType1_ = {},
                     OperandVar_t operand1_ = {}, OperandType_t operandType2_ = {},
                     OperandVar_t operand2_ = {} )
            : opcode( opcode_ )
            , operationType( operationType_ )
            , operandType1( operandType1_ )
            , operand1( operand1_ )
            , operandType2( operandType2_ )
            , operand2( operand2_ ) {
        }
    };
    struct MicroOperation_t {
        MicroOperationType_t type;
        Operand_t operand1;
        Operand_t operand2;
        MicroOperation_t( MicroOperationType_t type_ = MicroOperationType_t::EMPTY,
                          Operand_t operand1_ = Operand_t::NONE, Operand_t operand2_ = Operand_t::NONE )
            : type( type_ )
            , operand1( operand1_ )
            , operand2( operand2_ ) {
        }
    };

protected:
    // Default values of registers are for DMG; register f is initialized in constructor
    uint8_t registers[8] { 0x0, 0x13, 0x0, 0xD8, 0x1, 0x4D, 0x1 }; //b,c,d,e,h,l,a,f
    uint8_t Z, W;                                                  // temporary registers
    uint16_t SP = 0xFFFE, PC = 0x100;
    Operation_t operation = Operation_t( 0x0, OperationType_t::NOP );
    Memory& mem;
    bool interruptMasterEnabled = false;
    bool halted                 = false;

public:
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

    unsigned execute( const Operation_t& op );
    void execute( const MicroOperation_t mop );
    unsigned handleInterrupts();
    void ld( const Operation_t& op );
    void ldh( const Operation_t& op );
    void pushToStack( uint16_t value );
    uint16_t popFromStack();
    bool isConditionMet( Operand_t condition );

    Operation_t decode();
    using MicroOperations_t = std::array<MicroOperation_t, 6>;
    MicroOperations_t decode( int a );
    //helpers
    MicroOperations_t decodeBlock0( const uint8_t opcode );
    MicroOperations_t decodeBlock2( const uint8_t opcode );
    MicroOperations_t decodeBlock3( const uint8_t opcode );
    MicroOperations_t decodeCB( const uint8_t opcodeFirstByte );
    // clang-format off
    bool isPHL( Operand_t operand ) { return operand == Operand_t::phl; }
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
    static inline unsigned getCycles( uint16_t opcode, bool branchTaken ) {
        if( opcode <= 0xFF )
            return branchTaken ? cycles::opcodeCyclesBranched[opcode] : cycles::opcodeCycles[opcode];
        else
            return cycles::opcodeCyclesCb[opcode & 0xFF];
    }

    virtual void handleJoypad() = 0;
    void logOperation( Operation_t op, unsigned cycles );

public:
    CoreCpu( Memory& mem_ );
    virtual ~CoreCpu() = default;
    unsigned tick();
};
