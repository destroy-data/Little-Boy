#include "dummy_types.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

// Different implementation based on different source compared to CPU

namespace cycles {
// source: https://rgbds.gbdev.io/docs/v0.9.2/gbz80.7
constexpr unsigned NOP = 4;

constexpr unsigned LD_R16_IMM16 = 12;
constexpr unsigned LD_R16MEM_A  = 8;
constexpr unsigned LD_A_R16MEM  = 8;
constexpr unsigned LD_IMM16_SP  = 20;

constexpr unsigned INC_R16    = 8;
constexpr unsigned DEC_R16    = 8;
constexpr unsigned ADD_HL_R16 = 8;

constexpr unsigned INC_R8  = 4;
constexpr unsigned INC_pHL = 12;
constexpr unsigned DEC_R8  = 4;
constexpr unsigned DEC_pHL = 12;

constexpr unsigned LD_R8_IMM8  = 8;
constexpr unsigned LD_pHL_IMM8 = 12;

constexpr unsigned RLCA = 4;
constexpr unsigned RRCA = 4;
constexpr unsigned RLA  = 4;
constexpr unsigned RRA  = 4;
constexpr unsigned DAA  = 4;
constexpr unsigned CPL  = 4;
constexpr unsigned SCF  = 4;
constexpr unsigned CCF  = 4;

constexpr unsigned JR_IMM8              = 12;
constexpr unsigned JR_COND_IMM8_TAKEN   = 12;
constexpr unsigned JR_COND_IMM8_UNTAKEN = 8;

constexpr unsigned STOP = 4;

constexpr unsigned LD_R8_R8  = 4;
constexpr unsigned LD_R8_pHL = 8;
constexpr unsigned LD_pHL_R8 = 8;
constexpr unsigned HALT      = 4;

constexpr unsigned ARITHMETIC_A_R8   = 4;
constexpr unsigned ARITHMETIC_A_IMM8 = 8;
constexpr unsigned ARITH_A_pHL       = 8;

constexpr unsigned LOGIC_A_R8   = 4;
constexpr unsigned LOGIC_A_pHL  = 8;
constexpr unsigned LOGIC_A_IMM8 = 8;

constexpr unsigned CP_A_R8   = 4;
constexpr unsigned CP_A_pHL  = 8;
constexpr unsigned CP_A_IMM8 = 8;

constexpr unsigned RET_COND_TAKEN          = 20;
constexpr unsigned RET_COND_UNTAKEN        = 8;
constexpr unsigned RET                     = 16;
constexpr unsigned RETI                    = 16;
constexpr unsigned JP_COND_IMM16_TAKEN     = 16;
constexpr unsigned JP_COND_IMM16_UNTAKEN   = 12;
constexpr unsigned JP_IMM16                = 16;
constexpr unsigned JP_HL                   = 4;
constexpr unsigned CALL_COND_IMM16_TAKEN   = 24;
constexpr unsigned CALL_COND_IMM16_UNTAKEN = 12;
constexpr unsigned CALL_IMM16              = 24;
constexpr unsigned RST_TGT3                = 16;

constexpr unsigned POP_R16STK  = 12;
constexpr unsigned PUSH_R16STK = 16;

constexpr unsigned LDH_pR8_A   = 8;
constexpr unsigned LDH_pIMM8_A = 12;
constexpr unsigned LD_pIMM16_A = 16;
constexpr unsigned LDH_A_pR8   = 8;
constexpr unsigned LDH_A_pIMM8 = 12;
constexpr unsigned LD_A_pIMM16 = 16;

constexpr unsigned ADD_SP_IMM8        = 16;
constexpr unsigned LD_HL_SP_PLUS_IMM8 = 12;
constexpr unsigned LD_SP_HL           = 8;

constexpr unsigned DI = 4;
constexpr unsigned EI = 4;

constexpr unsigned CB_OP_R8  = 8;
constexpr unsigned CB_OP_pHL = 16;

constexpr unsigned BIT_B3_R8      = 8;
constexpr unsigned BIT_B3_pHL     = 12;
constexpr unsigned RES_SET_B3_R8  = 8;
constexpr unsigned RES_SET_B3_pHL = 16;
} // namespace cycles

/* clang-format off */
unsigned getCyclesAlternative( uint16_t opcode, bool branchTaken ) {
    switch( opcode ) {
    // Block 0
    case 0x00:
        return cycles::NOP;
    case 0x01:    case 0x11:    case 0x21:    case 0x31:
        return cycles::LD_R16_IMM16;
    case 0x02:    case 0x12:    case 0x22:    case 0x32:
        return cycles::LD_R16MEM_A;
    case 0x03:    case 0x13:    case 0x23:    case 0x33:
        return cycles::INC_R16;
    case 0x04:    case 0x0C:    case 0x14:    case 0x1C:    case 0x24:    case 0x2C:    case 0x3C:
        return cycles::INC_R8;
    case 0x34:
        return cycles::INC_pHL;
    case 0x05:    case 0x0D:    case 0x15:    case 0x1D:    case 0x25:    case 0x2D:    case 0x3D:
        return cycles::DEC_R8;
    case 0x35:
        return cycles::DEC_pHL;
    case 0x06:    case 0x0E:    case 0x16:    case 0x1E:    case 0x26:    case 0x2E:    case 0x3E:
        return cycles::LD_R8_IMM8;
    case 0x36:
        return cycles::LD_pHL_IMM8;
    case 0x07:
        return cycles::RLCA;
    case 0x08:
        return cycles::LD_IMM16_SP;
    case 0x09:    case 0x19:    case 0x29:    case 0x39:
        return cycles::ADD_HL_R16;
    case 0x0A:    case 0x1A:    case 0x2A:    case 0x3A:
        return cycles::LD_A_R16MEM;
    case 0x0B:    case 0x1B:    case 0x2B:    case 0x3B:
        return cycles::DEC_R16;
    case 0x0F:
        return cycles::RRCA;
    case 0x10:
        return cycles::STOP;
    case 0x17:
        return cycles::RLA;
    case 0x18:
        return cycles::JR_IMM8;
    case 0x1F:
        return cycles::RRA;
    case 0x20:    case 0x28:    case 0x30:    case 0x38:
        return branchTaken ? cycles::JR_COND_IMM8_TAKEN : cycles::JR_COND_IMM8_UNTAKEN;
    case 0x27:
        return cycles::DAA;
    case 0x2F:
        return cycles::CPL;
    case 0x37:
        return cycles::SCF;
    case 0x3F:
        return cycles::CCF;

    // Block 1: LD instructions
    case 0x40:    case 0x41:    case 0x42:    case 0x43:    case 0x44:    case 0x45:    case 0x47:    case 0x48:
    case 0x49:    case 0x4A:    case 0x4B:    case 0x4C:    case 0x4D:    case 0x4F:    case 0x50:    case 0x51:
    case 0x52:    case 0x53:    case 0x54:    case 0x55:    case 0x57:    case 0x58:    case 0x59:    case 0x5A:
    case 0x5B:    case 0x5C:    case 0x5D:    case 0x5F:    case 0x60:    case 0x61:    case 0x62:    case 0x63:
    case 0x64:    case 0x65:    case 0x67:    case 0x68:    case 0x69:    case 0x6A:    case 0x6B:    case 0x6C:
    case 0x6D:    case 0x6F:    case 0x78:    case 0x79:    case 0x7A:    case 0x7B:    case 0x7C:    case 0x7D:
    case 0x7F:
        return cycles::LD_R8_R8;
    case 0x46:    case 0x4E:    case 0x56:    case 0x5E:    case 0x66:    case 0x6E:    case 0x7E:
        return cycles::LD_R8_pHL;
    case 0x70:    case 0x71:    case 0x72:    case 0x73:    case 0x74:    case 0x75:    case 0x77:
        return cycles::LD_pHL_R8;
    case 0x76:
        return cycles::HALT;

    // Block 2: Arithmetic and Logic
    // Arithmetic operations
    case 0x80:    case 0x81:    case 0x82:    case 0x83:    case 0x84:    case 0x85:    case 0x87:    case 0x88:
    case 0x89:    case 0x8A:    case 0x8B:    case 0x8C:    case 0x8D:    case 0x8F:    case 0x90:    case 0x91:
    case 0x92:    case 0x93:    case 0x94:    case 0x95:    case 0x97:    case 0x98:    case 0x99:    case 0x9A:
    case 0x9B:    case 0x9C:    case 0x9D:    case 0x9F:
        return cycles::ARITHMETIC_A_R8;
    case 0x86:    case 0x8E:    case 0x96:    case 0x9E:
        return cycles::ARITH_A_pHL;

    // Logical operations
    case 0xA0:    case 0xA1:    case 0xA2:    case 0xA3:    case 0xA4:    case 0xA5:    case 0xA7:    case 0xA8:
    case 0xA9:    case 0xAA:    case 0xAB:    case 0xAC:    case 0xAD:    case 0xAF:    case 0xB0:    case 0xB1:
    case 0xB2:    case 0xB3:    case 0xB4:    case 0xB5:    case 0xB7:
        return cycles::LOGIC_A_R8;
    case 0xA6:    case 0xAE:    case 0xB6:
        return cycles::LOGIC_A_pHL;

    // Compare operations
    case 0xB8:    case 0xB9:    case 0xBA:    case 0xBB:    case 0xBC:    case 0xBD:    case 0xBF:
        return cycles::CP_A_R8;
    case 0xBE:
        return cycles::CP_A_pHL;

    // Block 3
    case 0xC0:    case 0xC8:    case 0xD0:    case 0xD8:
        return branchTaken ? cycles::RET_COND_TAKEN : cycles::RET_COND_UNTAKEN;
    case 0xC1:    case 0xD1:    case 0xE1:    case 0xF1:
        return cycles::POP_R16STK;
    case 0xC2:    case 0xCA:    case 0xD2:    case 0xDA:
        return branchTaken ? cycles::JP_COND_IMM16_TAKEN : cycles::JP_COND_IMM16_UNTAKEN;
    case 0xC3:
        return cycles::JP_IMM16;
    case 0xC4:    case 0xCC:    case 0xD4:    case 0xDC:
        return branchTaken ? cycles::CALL_COND_IMM16_TAKEN : cycles::CALL_COND_IMM16_UNTAKEN;
    case 0xC5:    case 0xD5:    case 0xE5:    case 0xF5:
        return cycles::PUSH_R16STK;
    case 0xC6:    case 0xCE:    case 0xD6:    case 0xDE:
        return cycles::ARITHMETIC_A_IMM8;
    case 0xE6:    case 0xEE:    case 0xF6:
        return cycles::LOGIC_A_IMM8;
    case 0xFE:
        return cycles::CP_A_IMM8;
    case 0xC7:    case 0xCF:    case 0xD7:    case 0xDF:    case 0xE7:    case 0xEF:    case 0xF7:    case 0xFF:
        return cycles::RST_TGT3;
    case 0xC9:
        return cycles::RET;
    case 0xCD:
        return cycles::CALL_IMM16;
    case 0xD9:
        return cycles::RETI;
    case 0xE0:
        return cycles::LDH_pIMM8_A;
    case 0xE2:
        return cycles::LDH_pR8_A;
    case 0xE8:
        return cycles::ADD_SP_IMM8;
    case 0xE9:
        return cycles::JP_HL;
    case 0xEA:
        return cycles::LD_pIMM16_A;
    case 0xF0:
        return cycles::LDH_A_pIMM8;
    case 0xF2:
        return cycles::LDH_A_pR8;
    case 0xF3:
        return cycles::DI;
    case 0xF8:
        return cycles::LD_HL_SP_PLUS_IMM8;
    case 0xF9:
        return cycles::LD_SP_HL;
    case 0xFA:
        return cycles::LD_A_pIMM16;
    case 0xFB:
        return cycles::EI;

    // CB-prefixed instructions - RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL on r8
    case 0xCB00:  case 0xCB01:  case 0xCB02:  case 0xCB03:  case 0xCB04:  case 0xCB05:  case 0xCB07:  case 0xCB08:
    case 0xCB09:  case 0xCB0A:  case 0xCB0B:  case 0xCB0C:  case 0xCB0D:  case 0xCB0F:  case 0xCB10:  case 0xCB11:
    case 0xCB12:  case 0xCB13:  case 0xCB14:  case 0xCB15:  case 0xCB17:  case 0xCB18:  case 0xCB19:  case 0xCB1A:
    case 0xCB1B:  case 0xCB1C:  case 0xCB1D:  case 0xCB1F:  case 0xCB20:  case 0xCB21:  case 0xCB22:  case 0xCB23:
    case 0xCB24:  case 0xCB25:  case 0xCB27:  case 0xCB28:  case 0xCB29:  case 0xCB2A:  case 0xCB2B:  case 0xCB2C:
    case 0xCB2D:  case 0xCB2F:  case 0xCB30:  case 0xCB31:  case 0xCB32:  case 0xCB33:  case 0xCB34:  case 0xCB35:
    case 0xCB37:  case 0xCB38:  case 0xCB39:  case 0xCB3A:  case 0xCB3B:  case 0xCB3C:  case 0xCB3D:  case 0xCB3F:
        return cycles::CB_OP_R8;

    // CB-prefixed instructions - RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL on (HL)
    case 0xCB06:  case 0xCB0E:  case 0xCB16:  case 0xCB1E:  case 0xCB26:  case 0xCB2E:  case 0xCB36:  case 0xCB3E:
        return cycles::CB_OP_pHL;

    // CB-prefixed instructions - BIT b, r8
    case 0xCB40:  case 0xCB41:  case 0xCB42:  case 0xCB43:  case 0xCB44:  case 0xCB45:  case 0xCB47:  case 0xCB48:
    case 0xCB49:  case 0xCB4A:  case 0xCB4B:  case 0xCB4C:  case 0xCB4D:  case 0xCB4F:  case 0xCB50:  case 0xCB51:
    case 0xCB52:  case 0xCB53:  case 0xCB54:  case 0xCB55:  case 0xCB57:  case 0xCB58:  case 0xCB59:  case 0xCB5A:
    case 0xCB5B:  case 0xCB5C:  case 0xCB5D:  case 0xCB5F:  case 0xCB60:  case 0xCB61:  case 0xCB62:  case 0xCB63:
    case 0xCB64:  case 0xCB65:  case 0xCB67:  case 0xCB68:  case 0xCB69:  case 0xCB6A:  case 0xCB6B:  case 0xCB6C:
    case 0xCB6D:  case 0xCB6F:  case 0xCB70:  case 0xCB71:  case 0xCB72:  case 0xCB73:  case 0xCB74:  case 0xCB75:
    case 0xCB77:  case 0xCB78:  case 0xCB79:  case 0xCB7A:  case 0xCB7B:  case 0xCB7C:  case 0xCB7D:  case 0xCB7F:
        return cycles::BIT_B3_R8;

    // CB-prefixed instructions - BIT b, (HL)
    case 0xCB46:  case 0xCB4E:  case 0xCB56:  case 0xCB5E:  case 0xCB66:  case 0xCB6E:  case 0xCB76:  case 0xCB7E:
        return cycles::BIT_B3_pHL;

    // CB-prefixed instructions - RES b, r8 and SET b, r8
    case 0xCB80:  case 0xCB81:  case 0xCB82:  case 0xCB83:  case 0xCB84:  case 0xCB85:  case 0xCB87:  case 0xCB88:
    case 0xCB89:  case 0xCB8A:  case 0xCB8B:  case 0xCB8C:  case 0xCB8D:  case 0xCB8F:  case 0xCB90:  case 0xCB91:
    case 0xCB92:  case 0xCB93:  case 0xCB94:  case 0xCB95:  case 0xCB97:  case 0xCB98:  case 0xCB99:  case 0xCB9A:
    case 0xCB9B:  case 0xCB9C:  case 0xCB9D:  case 0xCB9F:  case 0xCBA0:  case 0xCBA1:  case 0xCBA2:  case 0xCBA3:
    case 0xCBA4:  case 0xCBA5:  case 0xCBA7:  case 0xCBA8:  case 0xCBA9:  case 0xCBAA:  case 0xCBAB:  case 0xCBAC:
    case 0xCBAD:  case 0xCBAF:  case 0xCBB0:  case 0xCBB1:  case 0xCBB2:  case 0xCBB3:  case 0xCBB4:  case 0xCBB5:
    case 0xCBB7:  case 0xCBB8:  case 0xCBB9:  case 0xCBBA:  case 0xCBBB:  case 0xCBBC:  case 0xCBBD:  case 0xCBBF:
    case 0xCBC0:  case 0xCBC1:  case 0xCBC2:  case 0xCBC3:  case 0xCBC4:  case 0xCBC5:  case 0xCBC7:  case 0xCBC8:
    case 0xCBC9:  case 0xCBCA:  case 0xCBCB:  case 0xCBCC:  case 0xCBCD:  case 0xCBCF:  case 0xCBD0:  case 0xCBD1:
    case 0xCBD2:  case 0xCBD3:  case 0xCBD4:  case 0xCBD5:  case 0xCBD7:  case 0xCBD8:  case 0xCBD9:  case 0xCBDA:
    case 0xCBDB:  case 0xCBDC:  case 0xCBDD:  case 0xCBDF:  case 0xCBE0:  case 0xCBE1:  case 0xCBE2:  case 0xCBE3:
    case 0xCBE4:  case 0xCBE5:  case 0xCBE7:  case 0xCBE8:  case 0xCBE9:  case 0xCBEA:  case 0xCBEB:  case 0xCBEC:
    case 0xCBED:  case 0xCBEF:  case 0xCBF0:  case 0xCBF1:  case 0xCBF2:  case 0xCBF3:  case 0xCBF4:  case 0xCBF5:
    case 0xCBF7:  case 0xCBF8:  case 0xCBF9:  case 0xCBFA:  case 0xCBFB:  case 0xCBFC:  case 0xCBFD:  case 0xCBFF:
        return cycles::RES_SET_B3_R8;

    // CB-prefixed instructions - RES b, (HL) and SET b, (HL)
    case 0xCB86:  case 0xCB8E:  case 0xCB96:  case 0xCB9E:  case 0xCBA6:  case 0xCBAE:  case 0xCBB6:  case 0xCBBE:
    case 0xCBC6:  case 0xCBCE:  case 0xCBD6:  case 0xCBDE:  case 0xCBE6:  case 0xCBEE:  case 0xCBF6:  case 0xCBFE:
        return cycles::RES_SET_B3_pHL;

    default:
        return 0;
    }
}
/* clang-format on */

TEST_CASE( "instruction timings", "[cpu][timings]" ) {
}
