#pragma once

namespace cycles {
// source: https://rgbds.gbdev.io/docs/v0.9.2/gbz80.7
constexpr unsigned NOP = 4;

constexpr unsigned LD_R16_IMM16 = 16;
constexpr unsigned LD_R16MEM_A = 8;
constexpr unsigned LD_A_R16MEM = 8;
constexpr unsigned LD_IMM16_SP = 20;

constexpr unsigned INC_R16 = 8;
constexpr unsigned DEC_R16 = 8;
constexpr unsigned ADD_HL_R16 = 8;

constexpr unsigned INC_R8 = 4;
constexpr unsigned INC_pHL = 12;
constexpr unsigned DEC_R8 = 4;
constexpr unsigned DEC_pHL = 12;

constexpr unsigned LD_R8_IMM8 = 8;
constexpr unsigned LD_pHL_IMM8 = 12;

constexpr unsigned RLCA = 4;
constexpr unsigned RRCA = 4;
constexpr unsigned RLA = 4;
constexpr unsigned RRA = 4;
constexpr unsigned DAA = 4;
constexpr unsigned CPL = 4;
constexpr unsigned SCF = 4;
constexpr unsigned CCF = 4;

constexpr unsigned JR_IMM8 = 12;
constexpr unsigned JR_COND_IMM8_TAKEN = 12;
constexpr unsigned JR_COND_IMM8_UNTAKEN = 8;

constexpr unsigned STOP = 4;

constexpr unsigned LD_R8_R8 = 4;
constexpr unsigned LD_pHL_R8 = 8;
constexpr unsigned HALT = 4;

constexpr unsigned ADD_A_R8 = 4;
constexpr unsigned ADD_A_pHL = 8;
constexpr unsigned ADC_A_R8 = 4;
constexpr unsigned ADC_A_pHL = 8;
constexpr unsigned SUB_A_R8 = 4;
constexpr unsigned SUB_A_pHL = 8;
constexpr unsigned SBC_A_R8 = 4;
constexpr unsigned SBC_A_pHL = 8;
constexpr unsigned AND_A_R8 = 4;
constexpr unsigned AND_A_pHL = 4;
constexpr unsigned XOR_A_R8 = 4;
constexpr unsigned XOR_A_pHL = 8;
constexpr unsigned OR_A_R8 = 4;
constexpr unsigned OR_A_pHL = 8;
constexpr unsigned CP_A_R8 = 4;
constexpr unsigned CP_A_pHL = 8;

constexpr unsigned ADD_A_IMM8 = 8;
constexpr unsigned ADC_A_IMM8 = 8;
constexpr unsigned SUB_A_IMM8 = 8;
constexpr unsigned SBC_A_IMM8 = 8;
constexpr unsigned AND_A_IMM8 = 8;
constexpr unsigned XOR_A_IMM8 = 8;
constexpr unsigned OR_A_IMM8 = 8;
constexpr unsigned CP_A_IMM8 = 8;

constexpr unsigned RET_COND_TAKEN = 20;
constexpr unsigned RET_COND_UNTAKEN = 8;
constexpr unsigned RET = 4;
constexpr unsigned RETI = 4;
constexpr unsigned JP_COND_IMM16_TAKEN = 16;
constexpr unsigned JP_COND_IMM16_UNTAKEN = 12;
constexpr unsigned JP_IMM16 = 16;
constexpr unsigned JP_HL = 4;
constexpr unsigned CALL_COND_IMM16_TAKEN = 24;
constexpr unsigned CALL_COND_IMM16_UNTAKEN = 12;
constexpr unsigned CALL_IMM16 = 24;
constexpr unsigned RST_TGT3 = 16;

constexpr unsigned POP_R16STK = 12;
constexpr unsigned PUSH_R16STK = 16;

constexpr unsigned LDH_pR8_A = 8;
constexpr unsigned LDH_pIMM8_A = 12;
constexpr unsigned LD_pIMM16_A = 16;
constexpr unsigned LDH_A_pR8 = 8;
constexpr unsigned LDH_A_pIMM8 = 12;
constexpr unsigned LD_A_pIMM16 = 16;

constexpr unsigned ADD_SP_IMM8 = 16;
constexpr unsigned LD_HL_SP_PLUS_IMM8 = 12;
constexpr unsigned LD_SP_HL = 8;

constexpr unsigned DI = 4;
constexpr unsigned EI = 4;

constexpr unsigned RLC_R8 = 8;
constexpr unsigned RLC_pHL = 16;
constexpr unsigned RRC_R8 = 8;
constexpr unsigned RRC_pHL = 16;
constexpr unsigned RL_R8 = 8;
constexpr unsigned RL_pHL = 16;
constexpr unsigned RR_R8 = 8;
constexpr unsigned RR_pHL = 16;
constexpr unsigned SLA_R8 = 8;
constexpr unsigned SLA_pHL = 16;
constexpr unsigned SRA_R8 = 8;
constexpr unsigned SRA_pHL = 16;
constexpr unsigned SWAP_R8 = 8;
constexpr unsigned SWAP_pHL = 16;
constexpr unsigned SRL_R8 = 8;
constexpr unsigned SRL_pHL = 16;

constexpr unsigned BIT_B3_R8 = 8;
constexpr unsigned BIT_B3_pHL = 12;
constexpr unsigned RES_B3_R8 = 8;
constexpr unsigned RES_B3_pHL = 12;
constexpr unsigned SET_B3_R8 = 8;
constexpr unsigned SET_B3_pHL = 16;
} // namespace cycles
