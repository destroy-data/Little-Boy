#include "cartridge_impls/cartridge.hpp"
#include "core/logging.hpp"
#include <format>
#include <utility>

NoMBCCartridge::NoMBCCartridge( std::vector<uint8_t>&& rom_ ) : CoreCartridge( std::move( rom_ ) ) {};

uint8_t NoMBCCartridge::read( const uint16_t address ) {
    logDebug( 0, std::format( "Trying to read at address 0x{:04X}", address ) );

    if( romStartAddress <= address && address < ( romStartAddress + romBankSize * romBankCount ) ) {
        unsigned bankIndex  = address / romBankSize;
        uint16_t bankOffset = address % romBankSize;

        auto returnValue = romBanks[bankIndex][bankOffset];
        logInfo( 0, std::format( "Read value 0x{:02X} from ROM bank {} at offset 0x{:04X}", returnValue,
                                 bankIndex, bankOffset ) );
        return returnValue;
    }

    if( ramBankCount <= 0 ) {
        logWarning( 0, std::format( "No RAM banks available. Returning 0x{:02X}", 0xFF ) );
        return 0xFF;
    }

    if( ramStartAddress <= address && address < ( ramStartAddress + ramBankSize ) ) {
        auto returnValue = ramBanks[0][address - ramStartAddress];
        logInfo( 0, std::format( "Read value 0x{:02X} from RAM bank {} at address 0x{:04X}", 0, returnValue,
                                 address ) );
        return returnValue;
    }

    return 0xFF;
}

void NoMBCCartridge::write( const uint16_t address, const uint8_t value ) {
    logDebug( 0, std::format( "Trying to write value 0x{:02X} to address 0x{:04X}", value, address ) );

    if( ramBankCount <= 0 ) {
        logWarning( 0, "No RAM banks available. Write operation ignored." );
        return;
    }

    if( address < ramStartAddress || address >= ( ramStartAddress + ramBankSize ) ) {
        logWarning( 0, std::format( "Invalid address 0x{:04X}", address ) );
        return;
    }

    // Only first RAM bank is supported in NoMBC
    ramBanks[0][address - ramStartAddress] = value;
    logDebug( 0, std::format( "Wrote value 0x{:02X} to address 0x{:04X}", value, address ) );
}