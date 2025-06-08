#include "cartridge_impls/cartridge.hpp"
#include "core/logging.hpp"
#include <format>
#include <utility>

NoMBCCartridge::NoMBCCartridge( std::vector<uint8_t>&& rom_ ) : CoreCartridge( std::move( rom_ ) ) {};

uint8_t NoMBCCartridge::read( const uint16_t address ) {
    logDebug( std::format( "Trying to read at address {}", toHex( address ) ) );

    if( romStartAddress <= address && address < ( romStartAddress + romBankSize * romBankCount ) ) {
        unsigned bankIndex  = address / romBankSize;
        uint16_t bankOffset = address % romBankSize;

        auto returnValue = romBanks[bankIndex][bankOffset];
        logInfo( std::format( "Read value {} from ROM bank {} at offset {}", toHex( returnValue ), bankIndex,
                              toHex( bankOffset ) ) );
        return returnValue;
    }

    if( ramBankCount <= 0 ) {
        logWarning( 0, std::format( "No RAM banks available. Returning {}", toHex( invalidReadValue ) ) );
        return invalidReadValue;
    }

    if( ramStartAddress <= address && address < ( ramStartAddress + ramBankSize ) ) {
        auto returnValue = ramBanks[0][address - ramStartAddress];
        logInfo( std::format( "Read value {} from RAM bank {} at address {}", toHex( returnValue ), 0,
                              toHex( address ) ) );
        return returnValue;
    }

    logWarning( 0, std::format( "Address out of supported range. Returning {}", toHex( invalidReadValue ) ) );
    return invalidReadValue;
}

void NoMBCCartridge::write( const uint16_t address, const uint8_t value ) {
    logDebug( std::format( "Trying to write value {} to address {}", toHex( value ), toHex( address ) ) );

    if( ramBankCount <= 0 ) {
        logWarning( 0, "No RAM banks available. Write operation ignored." );
        return;
    }

    if( address < ramStartAddress || address >= ( ramStartAddress + ramBankSize ) ) {
        logWarning( 0, std::format( "Invalid address {}", toHex( address ) ) );
        return;
    }

    // Only first RAM bank is supported in NoMBC
    ramBanks[0][address - ramStartAddress] = value;
    logDebug( std::format( "Wrote value {} to address {}", toHex( value ), toHex( address ) ) );
}
