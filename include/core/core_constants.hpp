#pragma once
#include <cstdint>

namespace addr {
// Numbers in names describe bank numbers
constexpr uint16_t rom00                   = 0;
constexpr uint16_t rom0N                   = 0x4000; //optional switchable bank via mapper
constexpr uint16_t videoRam                = 0x8000; //in CGB mode switchable bank 0/1
constexpr uint16_t externalRam             = 0xA000; //optional switchable bank
constexpr uint16_t workRam00               = 0xC000;
constexpr uint16_t workRam0N               = 0xD000; //in CGB mode swichable bank 1-7
constexpr uint16_t echoRam00               = 0xE000; //echo of WORK_RAM_00
constexpr uint16_t echoRam0N               = 0xF000; //echo of WORK_RAM_0N
constexpr uint16_t objectAttributeMemory   = 0xFE00;
constexpr uint16_t notUsable               = 0xFEA0;
constexpr uint16_t ioRegisters             = 0xFF00;
constexpr uint16_t highRam                 = 0xFF80;
constexpr uint16_t interruptEnableRegister = 0xFFFF;
// PPU
constexpr uint16_t tileDataBlock0 = 0x8000;
constexpr uint16_t tileDataBlock1 = 0x8800;
constexpr uint16_t tileDataBlock2 = 0x9000;
constexpr uint16_t tileMap1       = 0x9800;
constexpr uint16_t tileMap2       = 0x9C00;
// IO ranges
constexpr uint16_t joypadInput            = 0xFF00;
constexpr uint16_t serialTransfer         = 0xFF01;
constexpr uint16_t serialTransferEnd      = 0xFF02;
constexpr uint16_t timer                  = 0xFF04;
constexpr uint16_t timerEnd               = 0xFF07;
constexpr uint16_t interruptFlag          = 0xFF0F;
constexpr uint16_t audio                  = 0xFF10;
constexpr uint16_t audioEnd               = 0xFF26;
constexpr uint16_t wavePattern            = 0xFF30;
constexpr uint16_t wavePatternEnd         = 0xFF3F;
constexpr uint16_t lcd                    = 0xFF40;
constexpr uint16_t lcdEnd                 = 0xFF4B;
constexpr uint16_t vramBankSelect         = 0xFF4F; //CGB only
constexpr uint16_t bootRomDisableRegister = 0xFF50;
// IO ranges - CGB only
constexpr uint16_t vramDma        = 0xFF51;
constexpr uint16_t vramDmaEnd     = 0xFF55;
constexpr uint16_t paletes        = 0xFF68;
constexpr uint16_t paletesEnd     = 0xFF6B;
constexpr uint16_t wramBankSelect = 0xFF70;
// Registers
constexpr uint16_t divider      = 0xFF04;
constexpr uint16_t timerCounter = 0xFF05;
constexpr uint16_t timerModulo  = 0xFF06;
constexpr uint16_t timerControl = 0xFF07;
constexpr uint16_t lcdControl   = 0xFF40;
constexpr uint16_t lcdStatus    = 0xFF41;
constexpr uint16_t bgScrollY    = 0xFF42;
constexpr uint16_t bgScrollX    = 0xFF43;
constexpr uint16_t lcdY         = 0xFF44;
constexpr uint16_t lyc          = 0xFF45;
constexpr uint16_t winY         = 0xFF4A;
constexpr uint16_t winX         = 0xFF4B;
// Registers - non-CGB mode only
constexpr uint16_t bgPalette      = 0xFF47;
constexpr uint16_t objectPalette0 = 0xFF48;
constexpr uint16_t objectPalette1 = 0xFF49;
// Registers - CGB mode only
constexpr uint16_t bgPaletteSpec  = 0xFF68;
constexpr uint16_t bgPaletteData  = 0xFF69;
constexpr uint16_t objPaletteSpec = 0xFF6A;
constexpr uint16_t objPaletteData = 0xFF6B;
constexpr uint16_t key1           = 0xFF4D;

//Cartridge related
constexpr uint16_t entryPointStart = 0x0100;
constexpr uint16_t entryPointEnd   = 0x0103;

constexpr uint16_t logoStart = 0x0104;
constexpr uint16_t logoEnd   = 0x0133;

constexpr uint16_t titleStart = 0x0134;
constexpr uint16_t titleEnd   = 0x0143;

constexpr uint16_t manufacturerCodeStart = 0x013F;
constexpr uint16_t manufacturerCodeEnd   = 0x0142;

constexpr uint16_t cgbFlag = 0x0143;

constexpr uint16_t newLicenseeCodeStart = 0x0144;
constexpr uint16_t newLicenseeCodeEnd   = 0x0145;

constexpr uint16_t sgbFlag         = 0x0146;
constexpr uint16_t cartridgeType   = 0x0147;
constexpr uint16_t romSize         = 0x0148;
constexpr uint16_t ramSize         = 0x0149;
constexpr uint16_t destinationCode = 0x014A;
constexpr uint16_t oldLicenseeCode = 0x014B;
constexpr uint16_t maskRomVersion  = 0x014C;

constexpr uint16_t headerChecksum      = 0x014D;
constexpr uint16_t globalChecksumStart = 0x014E;
constexpr uint16_t globalChecksumEnd   = 0x014F;
} // namespace addr

namespace size {
constexpr uint16_t oam      = 160;
constexpr uint16_t tile     = 16;
constexpr uint16_t videoRam = 8192;
} // namespace size

// other
namespace constant {
static constexpr unsigned tickrate     = 4'194'304;
static constexpr float oscillatoryTime = 1. / tickrate;
} // namespace constant
