#include "cartridge_impls/cartridge.hpp"
#include "core/cartridge.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <format>

MBC2Cartridge::MBC2Cartridge( std::vector<uint8_t>&& rom_ ) : CoreCartridge( std::move( rom_ ) ) {
    logDebug( "MBC2Cartridge constructor" );

    if( getRomSize() > RomSize::_256KiB ) {
        logError( 0, "ROM size greater than 256 KiB is not supported by MBC2 cartridge." );
        return;
    }

    if( getRamSize() > RamSize::_0KiB ) {
        logError( 0, "RAM size other than 0 KiB is not supported by MBC2 cartridge." );
        return;
    }

    logDebug( std::format( "Initialize RAM consisting of {} half-bytes", halfByteRamSize ) );
    ramBanks = std::vector<std::vector<uint8_t>>( 1, std::vector<uint8_t>( halfByteRamSize, 0 ) );
};

uint8_t MBC2Cartridge::read( const uint16_t address ) {
    logDebug( std::format( "Trying to read at address {}", toHex( address ) ) );

    if( isInPrimaryRomRange( address ) ) {
        const auto returnValue = romBanks[0][address];
        logInfo( std::format( "Read value {} from ROM bank 0 at address {}", toHex( returnValue ),
                              toHex( address ) ) );
        return returnValue;
    }

    if( isInSecondaryRomRange( address ) ) {
        const uint16_t offset  = address % romBankSize;
        const auto returnValue = romBanks[selectedRomBankRegister][offset];
        logInfo( std::format( "Read value {} from ROM bank {} at offset {}", toHex( returnValue ),
                              selectedRomBankRegister, toHex( offset ) ) );
        return returnValue;
    }

    if( isInRamRange( address ) ) {
        if( ramEnabled ) {
            const auto ramBankAddress = ( address - ramStartAddress ) % halfByteRamSize;
            const uint8_t returnValue = ramBanks[0][ramBankAddress] & halfByteMask;
            logInfo( std::format( "Read value {} from half-byte RAM at offset {}", toHex( returnValue ),
                                  toHex( ramBankAddress ) ) );
            return returnValue;
        }
        logWarning( 0, std::format( "RAM is not enabled. Invalid read from address {}. Returning {}",
                                    toHex( address ), toHex( invalidReadValue ) ) );
        return invalidReadValue;
    }

    logWarning( 0, std::format( "Read operation from address {} is not supported", toHex( address ) ) );
    return invalidReadValue;
}

void MBC2Cartridge::write( const uint16_t address, const uint8_t value ) {
    logDebug( std::format( "Trying to write value {} to address {}", toHex( value ), toHex( address ) ) );

    if( isInPrimaryRomRange( address ) ) {
        if( address & 0x100 ) { // 8th bit is set
            selectedRomBankRegister = value & halfByteMask;
            if( selectedRomBankRegister == 0 ) {
                selectedRomBankRegister = 1; // ROM bank 0 not allowed
            }

            logInfo( std::format( "Selected ROM bank register set to {:#06b}", selectedRomBankRegister ) );
            return;
        }

        if( value == ramEnableValue ) {
            ramEnabled = true;
            logInfo( "RAM enabled" );
            return;
        } else {
            ramEnabled = false;
            logInfo( "RAM disabled" );
            return;
        }
    }

    if( isInRamRange( address ) ) {
        if( ramEnabled ) {
            const uint16_t offset = ( address - ramStartAddress ) % halfByteRamSize;
            ramBanks[0][offset]   = value & halfByteMask;

            logInfo( std::format( "Wrote value {} to half-byte RAM offset {}", toHex( value ),
                                  toHex( offset ) ) );
            return;
        }
        logWarning( 0, "RAM is not enabled. Cannot write to RAM." );
        return;
    }

    logWarning( 0, std::format( "Write operation to address {} is not supported", toHex( address ) ) );
}
