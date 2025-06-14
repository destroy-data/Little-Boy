#include "cartridge_impls/cartridge.hpp"
#include "core/cartridge.hpp"
#include "core/logging.hpp"
#include <format>

MBC1Cartridge::MBC1Cartridge( std::vector<uint8_t>&& rom_ ) : CoreCartridge( std::move( rom_ ) ) {
    if( getRomSize() > RomSize::_512KiB && getRamSize() > RamSize::_8KiB ) {
        logError( 0, "Invalid ROM/RAM configuration for MBC1 cartridge." );
        return;
    }

    if( getRomSize() > RomSize::_2MiB ) {
        logError( 0, "ROM size greater than 2 MiB is not supported by MBC1 cartridge." );
        return;
    }

    if( getRamSize() > RamSize::_32KiB ) {
        logError( 0, "RAM size greater than 32 KiB is not supported by MBC1 cartridge." );
        return;
    }

    if( getRomSize() > RomSize::_512KiB ) {
        logInfo( "Using MBC1 alternate wiring" );
        alternateWiring = true;
    }

    if( getRomSize() == RomSize::_1MiB &&
        CoreCartridge::checkCopyRightHeader( addresses::secondMBCMRomBank ) ) {
        logInfo( "MBC1M cartridge detected." );
        isMBCM = true;
    }
}

unsigned MBC1Cartridge::getRomBankIndex( bool isPrimaryRom ) const {
    const auto romSize = getRomSize();
    if( romSize <= CoreCartridge::RomSize::_512KiB ) {
        if( isPrimaryRom ) {
            return 0;
        }

        unsigned bankIndex = selectedRomBankRegister & selectedRomBankBitMask;
        if( romSize > CoreCartridge::RomSize::_256KiB ) {
            return bankIndex == 0 ? 1 : bankIndex;
        }

        const bool isFifthBitSet = selectedRomBankRegister & 0b10000;
        bankIndex &= 0b1111;
        if( isFifthBitSet && bankIndex == 0 ) {
            return 0;
        }
        return bankIndex == 0 ? 1 : bankIndex;
    }

    if( isPrimaryRom ) {
        if( isMBCM ) {
            return isInBankingMode ? unsigned { bankingRegister } << 4 : 0u;
        }

        return isInBankingMode ? unsigned { bankingRegister } << 5 : 0u;
    }

    unsigned bankIndex = selectedRomBankRegister & selectedRomBankBitMask;
    if( isMBCM ) {
        const bool isFifthBitSet = selectedRomBankRegister & 0b10000;
        bankIndex &= selectedMBCMRomBankBitMask;
        if( ! isFifthBitSet && bankIndex == 0 ) {
            bankIndex = 1;
        }
        bankIndex |= ( unsigned { bankingRegister } << 4 );

        return bankIndex;
    }

    if( bankIndex == 0 ) {
        bankIndex = 1;
    }
    bankIndex |= ( unsigned { bankingRegister } << 5 );

    return bankIndex;
}

uint8_t MBC1Cartridge::readRom( const uint16_t address, bool isPrimaryRom ) const {
    uint16_t bankOffset = address % romBankSize;
    auto romBankIndex   = getRomBankIndex( isPrimaryRom );

    auto returnValue = romBanks[romBankIndex][bankOffset];
    logInfo( std::format( "Read value {} from ROM bank {} at offset {}", toHex( returnValue ), romBankIndex,
                          toHex( bankOffset ) ) );
    return returnValue;
}

uint8_t MBC1Cartridge::read( const uint16_t address ) {
    logDebug( std::format( "Trying to read at address {}", toHex( address ) ) );

    if( isInPrimaryRomRange( address ) ) {
        return readRom( address, true );
    }

    if( isInSecondaryRomRange( address ) ) {
        return readRom( address, false );
    }

    if( isInRamRange( address ) ) {
        if( getRamBankCount() == 0 ) {
            logWarning( 0, std::format( "No RAM banks available. Returning {}", toHex( invalidReadValue ) ) );
            return invalidReadValue;
        }

        if( ! ramEnabled ) {
            logWarning( 0, std::format( "RAM is not enabled. Returning {}", toHex( invalidReadValue ) ) );
            return invalidReadValue;
        }

        if( getRamSize() == RamSize::_8KiB ) {
            auto returnValue = ramBanks[0][address - ramStartAddress];
            logInfo( std::format( "Read value {} from RAM bank 0 at address {}", toHex( returnValue ),
                                  toHex( address ) ) );
            return returnValue;
        }

        // TODO: check if secondaryRegister is less or equal to ramBankCount?
        auto returnValue = ramBanks[bankingRegister][address - ramStartAddress];
        logInfo( std::format( "Read value {} from RAM bank {} at address {}", toHex( returnValue ),
                              bankingRegister, toHex( address ) ) );
        return returnValue;
    }

    logWarning( 0, std::format( "Address {} is not in RAM nor ROM range", toHex( address ) ) );
    return invalidReadValue;
}

void MBC1Cartridge::write( const uint16_t address, const uint8_t value ) {
    logDebug( std::format( "Trying to write value {} to address {}", toHex( value ), toHex( address ) ) );

    if( isInRamEnableRange( address ) ) {
        if( value == ramEnableValue ) {
            ramEnabled = true;
            logInfo( "RAM enabled" );
            return;
        }

        ramEnabled = false;
        logInfo( "RAM disabled" );
        return;
    }

    if( isInRomBankSelectRange( address ) ) {
        selectedRomBankRegister = value & 0b11111; // 5 bits for ROM bank selection
        logInfo( std::format( "ROM bank selection register set to {:#07b}", selectedRomBankRegister ) );
        return;
    }

    if( isInSecondaryRegisterRange( address ) ) {
        bankingRegister = value & 0b11; // 2 bits for secondary register
        logInfo( std::format( "Secondary register set to {:#04b}", bankingRegister ) );
        return;
    }

    if( isInBankingModeSelectRange( address ) ) {
        isInBankingMode = ( value & 0b1 ) == 1;
        logInfo( std::format( "Banking mode set to {}", isInBankingMode ? "Advanced" : "Simple" ) );
        return;
    }

    if( isInRamRange( address ) ) {
        if( getRamBankCount() == 0 ) {
            logWarning( 0, "No RAM banks available. Write operation ignored." );
            return;
        }

        if( ! ramEnabled ) {
            logWarning( 0, "RAM is not enabled. Write operation ignored." );
            return;
        }

        if( getRamSize() == RamSize::_8KiB ) {
            ramBanks[0][address - ramStartAddress] = value;
            logInfo( std::format( "Wrote value {} to RAM bank 0 at address {}", toHex( value ),
                                  toHex( address ) ) );
            return;
        }

        // TODO: check if secondaryRegister is less or equal to ramBankCount?
        unsigned ramBankIndex                             = isInBankingMode ? bankingRegister : 0;
        ramBanks[ramBankIndex][address - ramStartAddress] = value;
        logInfo( std::format( "Wrote value {} to RAM bank {} at address {}", toHex( value ), bankingRegister,
                              toHex( address ) ) );
        return;
    }

    logWarning( 0, std::format( "Write operation to address {} is not supported", toHex( address ) ) );
}
