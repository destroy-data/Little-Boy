
#include "raylib/cartridge.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>

RaylibCartridge::RaylibCartridge() {
    const auto romDir = std::filesystem::current_path() / "roms";
    auto dirFiles = std::filesystem::directory_iterator( romDir );
    for( const auto& dirFile: dirFiles ) {
        //Take the first file that have appropiate extension, ignore the rest
        for( const auto extension: romExtensions ) {
            if( dirFile.path().extension() == extension ) {
                logDebug( 0, std::format( "Found file: {} with size {}K", dirFile.path().string(),
                                          dirFile.file_size() / 1024 ) );
                std::ifstream file( dirFile.path(), std::ios::binary );
                const auto size = static_cast<std::streamsize>( dirFile.file_size() );

                std::vector<uint8_t> buffer( static_cast<size_t>( size ) );
                if( file.read( reinterpret_cast<char*>( buffer.data() ), size ) ) {
                    rom = std::move( buffer );
                    // by default bank01 is used in switachble bank
                    romBank0N = { &rom[0] + 16384, 16384 };
                }
                goto found;
            }
        }
    }
    logError( ErrorCode::CartridgeNotFound, "Didn't find any cartridge." );
    std::abort(); //TODO: something graceful, for example open filepicker.
found:
    //parse header
    if( rom.size() < 0x14F ) {
        logFatal( ErrorCode::InvalidCartridge,
                  "File too small, it's size is " + std::to_string( rom.size() ) + " bytes." );
    }
    uint8_t headerChecksum = 0;
    for( uint16_t address = 0x0134; address <= 0x014C; address++ ) {
        headerChecksum = static_cast<uint8_t>( headerChecksum - rom[address] - 1 );
    }
    if( headerChecksum != rom[0x14D] ) {
        logFatal( ErrorCode::RomHeaderChecksumMismatch,
                  std::format( "computed: {}, in rom: {}", headerChecksum, rom[0x14D] ) );
    }
}
