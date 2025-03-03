#include "raylib/Cartridge.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>

void Cartridge::RTC::updateRTC() {
    if( rtcHalted )
        return;

    std::time_t currentTime = std::time( nullptr );
    std::time_t elapsedSeconds = currentTime - rtcBaseTime;

    rtcRegs.seconds = static_cast<uint8_t>( ( elapsedSeconds % 60 ) );
    rtcRegs.minutes = static_cast<uint8_t>( ( elapsedSeconds / 60 ) % 60 );
    rtcRegs.hours = static_cast<uint8_t>( ( elapsedSeconds / 3600 ) % 24 );
    uint16_t days =
            static_cast<uint16_t>( ( elapsedSeconds / 86400 ) % 512 ); // 9-bit day counter (0-511)
    rtcRegs.dayLow = days & 0xFF;

    // Bit 0 of dayHigh is the upper bit of the day counter
    // Preserve the halt flag (bit 6)
    rtcRegs.dayHigh = ( rtcRegs.dayHigh & ( 1 << 6 ) ) | ( ( days > 255 ) ? 0x01 : 0x00 );

    // day counter carry bit
    if( elapsedSeconds / 86400 >= 512 ) {
        rtcRegs.dayHigh |= ( 1 << 7 );
    }
}

uint8_t Cartridge::RTC::readRTC( uint8_t reg ) {
    if( !rtcLatchedData )
        updateRTC();

    RTCRegisters& regs = rtcLatchedData ? latchedRtcRegs : rtcRegs;

    switch( reg ) {
    case 0x08:
        return regs.seconds;
    case 0x09:
        return regs.minutes;
    case 0x0A:
        return regs.hours;
    case 0x0B:
        return regs.dayLow;
    case 0x0C:
        return regs.dayHigh;
    default:
        return 0xFF; // Invalid RTC register
    }
}

void Cartridge::RTC::writeRTC( uint8_t reg, uint8_t value ) {
    switch( reg ) {
    case 0x08:
        rtcRegs.seconds = value & 0x3F; // 0-59
        break;
    case 0x09:
        rtcRegs.minutes = value & 0x3F; // 0-59
        break;
    case 0x0A:
        rtcRegs.hours = value & 0x1F; // 0-23
        break;
    case 0x0B:
        rtcRegs.dayLow = value;
        break;
    case 0x0C:
        rtcRegs.dayHigh = value & 0xC1; // Only bits 7, 6, and 0 can be written
        rtcHalted = ( value & 0x40 ) != 0;
        break;
    }

    if( reg >= 0x08 && reg <= 0x0C ) {
        int days = ( ( rtcRegs.dayHigh & 0x01 ) << 8 ) | rtcRegs.dayLow;
        std::time_t totalSeconds =
                rtcRegs.seconds + rtcRegs.minutes * 60 + rtcRegs.hours * 3600 + days * 86400;

        rtcBaseTime = std::time( nullptr ) - totalSeconds;
    }
}

void Cartridge::RTC::latchClockData( uint8_t value ) {
    if( !rtcLatchedData && value == 0x1 ) {
        updateRTC();
        latchedRtcRegs = rtcRegs;
        rtcLatchedData = true;
    } else if( !value ) {
        rtcLatchedData = false;
    }
}

uint8_t Cartridge::read( const uint16_t address ) {
    //TODO
    if( address < 0x4000 )
        return rom[address];
    if( address < 0x8000 )
        return romBank0N[address - 0x4000];
    if( address >= 0x8000 && address < 0xA000 )
        return 0xFF;
    if( address < 0xC000 ) {
        return ramBank[address];
    }
}


void Cartridge::write( const uint16_t address, uint8_t value ) {
    switch( static_cast<CartridgeType>( cartridgeType ) ) {
    case CartridgeType::MBC1:
    case CartridgeType::MBC1_RAM:
    case CartridgeType::MBC1_RAM_BATTERY:
        if( address < 0x2000 )
            enableRam = ( value & 0xF ) == 0xA;
        else if( address < 0x4000 )
            switchRomBank( value );
        else if( address < 0x6000 )
            switchRamBank( value );
        else if( address < 0x8000 )
            switchBankingMode( value );
        return;
    case CartridgeType::MBC2:
    case CartridgeType::MBC2_BATTERY:
        if( address < 0x4000 ) {
            if( address & 0x100 )
                switchRomBank( value );
            else
                enableRam = ( value & 0xF ) == 0xA;
        }
        return;
    case CartridgeType::MBC3:
    case CartridgeType::MBC3_RAM:
    case CartridgeType::MBC3_RAM_BATTERY:
    case CartridgeType::MBC3_TIMER_BATTERY:
    case CartridgeType::MBC3_TIMER_RAM_BATTERY:
        if( address < 0x2000 )
            enableRam = ( value & 0xF ) == 0xA;
        else if( address < 0x4000 )
            switchRomBank( value & 0x7F );
        else if( address < 0x6000 ) {
            ramBankNr = value;
            switchRamBank( value );
        } else if( address < 0x8000 ) {
            rtc.latchClockData( value );
        } else if( address >= 0xA000 && address < 0xC000 ) {
            // Write to RAM or RTC register
            if( enableRam ) {
                if( ramBankNr >= 0x08 && ramBankNr <= 0x0C ) {
                    rtc.writeRTC( ramBankNr, value );
                } else if( ramBank.size() > 0 ) {
                    ramBank[address - 0xA000] = value;
                }
            }
        }
        return;
    }
}

void Cartridge::switchBankingMode( uint8_t value ) {
    //TODO
}

void Cartridge::switchRomBank( uint8_t bank ) {
    //TODO after adding switching banking mode, make sure it interoperates with this
    // Make sure we don't exceed available ROM banks
    uint32_t numBanks = rom.size() / 16384;
    if( numBanks <= 1 )
        return;

    bank %= numBanks;
    if( bank == 0 )
        bank = 1; // Bank 0 is always mapped to 0000-3FFF

    uint32_t offset = bank * 16384;
    if( offset + 16384 <= rom.size() ) {
        romBank0N = std::span<uint8_t>( &rom[offset], 16384 );
    }
}

void Cartridge::switchRamBank( uint8_t bank ) {
    //TODO after adding switching banking mode, make sure it interoperates with this
    // Calculate RAM size based on header value
    uint32_t ramSize = 0;
    switch( this->ramSize ) {
    case 0x00:
        ramSize = 0;
        break; // No RAM
    case 0x01:
        ramSize = 2048;
        break; // 2 KB (one bank)
    case 0x02:
        ramSize = 8192;
        break; // 8 KB (one bank)
    case 0x03:
        ramSize = 32768;
        break; // 32 KB (4 banks of 8 KB each)
    case 0x04:
        ramSize = 131072;
        break; // 128 KB (16 banks of 8 KB each)
    case 0x05:
        ramSize = 65536;
        break; // 64 KB (8 banks of 8 KB each)
    }

    // If no RAM or only one bank, ignore bank switching
    if( ramSize <= 8192 )
        return;

    uint32_t numBanks = ramSize / 8192;
    bank %= numBanks;

    uint32_t offset = bank * 8192;
    if( offset + 8192 <= ramBank.size() ) {
        ramBank = std::span<uint8_t>( &ramBank[offset], 8192 );
    }
}

Cartridge::Cartridge() {
    const auto romDir = std::filesystem::current_path() / "roms";
    auto dirFiles = std::filesystem::directory_iterator( romDir );
    for( const auto& dirFile: dirFiles ) {
        //Take the first file that have appropiate extension, ignore the rest
        for( const auto extension: romExtensions ) {
            if( dirFile.path().extension() == extension ) {
                std::ifstream file( dirFile.path(), std::ios::binary );
                const auto size = static_cast<std::streamsize>( dirFile.file_size() );

                std::vector<uint8_t> buffer( static_cast<size_t>( size ) );
                if( file.read( reinterpret_cast<char*>( buffer.data() ), size ) ) {
                    rom = std::move( buffer );
                    // by default bank01 is used in switachble bank
                    romBank0N = { &rom[0] + 16384, 16384 };
                }
                break;
            }
        }
        //parse header
        if( rom.size() < 0x14F ) {
            //ERRTODO
        }
        cbgFlag = rom[0x143];
        cartridgeType = rom[0x147];
        romSize = rom[0x148];
        ramSize = rom[0x149];
        uint8_t checksum = 0;
        for( uint16_t address = 0x0134; address <= 0x014C; address++ ) {
            checksum = checksum - rom[address] - 1;
        }
        if( checksum != rom[0x14D] ) {
            //ERRTODO
        }
    }
}
