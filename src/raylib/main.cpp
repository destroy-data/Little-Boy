#include "core/cartridge.hpp"
#include "core/logging.hpp"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <raylib.h>
#include <vector>

int main() {
    // clang-format off
    auto romPath = tinyfd_openFileDialog(
        "Choose cartridge",    // dialog title
        "",              // default directory path
        2,                     // number of filter patterns
        CoreCartridge::cartridgeFilePatterns, // filter patterns
        NULL,                  // signle filter description
        0                      // allow multiple selection
    );
    // clang-format on

    std::ifstream romFile( romPath, std::ios::binary );
    if( !romFile.good() ) {
        logFatal( 0, "Failed to open ROM file: " + std::string( romPath ) );
        return 1;
    }

    auto dataSize = std::filesystem::file_size( romPath );
    std::vector<uint8_t> romData( dataSize );
    romFile.read( reinterpret_cast<char*>( romData.data() ), dataSize );
    romFile.close();


    CoreCartridge::CartridgeType type = static_cast<CoreCartridge::CartridgeType>( romData[0x147] );
    auto cartridge = CoreCartridge::create( type, std::move( romData ) );

    logDebug( 0, std::format( "Read cartridge type byte: 0x{:02X}", cartridge->read( addr::cartridgeType ) ) );
    logDebug( 0, std::format( "Read ROM size byte: 0x{:02X}", cartridge->read( addr::romSize ) ) );
    logDebug( 0, std::format( "Read RAM size byte: 0x{:02X}", cartridge->read( addr::ramSize ) ) );

    for( uint16_t logoAddress = addr::logoStart; logoAddress <= addr::logoEnd; ++logoAddress ) {
        logDebug( 0, std::format( "Read logo byte at address 0x{:04X}: 0x{:02X}", logoAddress,
                                  cartridge->read( logoAddress ) ) );
    }

    logDebug( 0, std::format( "Try to read from ROM bank 1. Read from address 0x{:04X}: 0x{:02X}", 0x4000,
                              cartridge->read( 0x4000 ) ) );

    logDebug( 0,
              std::format( "Try to read from RAM (which does not exist). Read from address 0x{:04X}: 0x{:02X}",
                           0xA000, cartridge->read( 0xA000 ) ) );

    return 0;
}
