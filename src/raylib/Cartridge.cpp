#include "raylib/Cartridge.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>


uint8_t Cartridge::read( const uint16_t ) {
    ;
}


void Cartridge::write( const uint16_t address, uint8_t value ) {
    ;
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
                    /* worked! */
                    romData = std::move( buffer );
                }
                break;
            }
        }
        if( romData.empty() ) {
            //ERRTODO
        }
    }
}
