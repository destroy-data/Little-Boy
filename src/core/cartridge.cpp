#include "core/cartridge.hpp"

#include "core/core_constants.hpp"
#include "core/logging.hpp"
#include <format>
#include <type_traits>
#include <utility>

bool CoreCartridge::isValidRomSize( const RomSizeByte size ) {
    switch( size ) {
        using enum RomSizeByte;
    case _32KiB:
    case _64KiB:
    case _128KiB:
    case _256KiB:
    case _512KiB:
    case _1MiB:
    case _2MiB:
    case _4MiB:
    case _8MiB:
        return true;

    case _1_1MiB:
        logError( 0, "ROM size 1.1MiB is not supported." );
        break;
    case _1_2MiB:
        logError( 0, "ROM size 1.2MiB is not supported." );
        break;
    case _1_5MiB:
        logError( 0, "ROM size 1.5MiB is not supported." );
        break;
    default:
        break;
    }
    logError( 0, std::format( "ROM size unknown. Byte: {}",
                              toHex( static_cast<std::underlying_type_t<RomSizeByte>>( size ) ) ) );
    return false;
}

void CoreCartridge::setRomSize( const RomSizeByte size ) {
    switch( size ) {
        using enum RomSizeByte;
    case _32KiB:
        romSize = RomSize::_32KiB;
        break;
    case _64KiB:
        romSize = RomSize::_64KiB;
        break;
    case _128KiB:
        romSize = RomSize::_128KiB;
        break;
    case _256KiB:
        romSize = RomSize::_256KiB;
        break;
    case _512KiB:
        romSize = RomSize::_512KiB;
        break;
    case _1MiB:
        romSize = RomSize::_1MiB;
        break;
    case _2MiB:
        romSize = RomSize::_2MiB;
        break;
    case _4MiB:
        romSize = RomSize::_4MiB;
        break;
    case _8MiB:
        romSize = RomSize::_8MiB;
        break;
    default:
        std::unreachable();
    }
}

void CoreCartridge::initRom( const RomSizeByte size ) {
    if( ! isValidRomSize( size ) ) {
        logError( 0, "Invalid ROM size. Failed to initialize ROM." );
        return;
    }
    setRomSize( size );

    romBanks.clear();
    romBanks.reserve( getRomBankCount() );

    for( unsigned i = 0; i < getRomBankCount(); ++i ) {
        auto offset = i * romBankSize;
        romBanks.emplace_back( rom.data() + offset, romBankSize );
        logDebug( std::format( "Initialized ROM bank {} at offset {} of size {}", i, toHex( offset ),
                               toHex( romBankSize ) ) );
    }
    logInfo( std::format( "Initialized {} ROM banks of size {} bytes", romBanks.size(),
                          toHex( romBankSize ) ) );
}

bool CoreCartridge::isValidRamSize( const RamSizeByte size ) {
    switch( size ) {
        using enum RamSizeByte;
    case _0KiB:
    case _8KiB:
    case _32KiB:
    case _64KiB:
    case _128KiB:
        return true;

    case _2KiB:
        logError( 0, "RAM size 2KiB is not supported" );
        break;
    default:
        break;
    }
    logError( 0, std::format( "RAM size unknown. Byte: {}",
                              toHex( static_cast<std::underlying_type_t<RamSizeByte>>( size ) ) ) );
    return false;
}

void CoreCartridge::setRamSize( const CoreCartridge::RamSizeByte size ) {
    switch( size ) {
        using enum RamSizeByte;
    case _0KiB:
        ramSize = RamSize::_0KiB;
        break;
    case _8KiB:
        ramSize = RamSize::_8KiB;
        break;
    case _32KiB:
        ramSize = RamSize::_32KiB;
        break;
    case _64KiB:
        ramSize = RamSize::_64KiB;
        break;
    case _128KiB:
        ramSize = RamSize::_128KiB;
        break;
    default:
        std::unreachable();
    }
}

void CoreCartridge::initRam( const CoreCartridge::RamSizeByte size ) {
    if( ! isValidRamSize( size ) ) {
        logError( 0, "Invalid RAM size. Failed to initialize RAM." );
        return;
    }
    setRamSize( size );

    ramBanks = std::vector<std::vector<uint8_t>>( getRamBankCount(), std::vector<uint8_t>( ramBankSize, 0 ) );
    logInfo( std::format( "Initialized {} RAM banks of size {} bytes", ramBanks.size(),
                          toHex( ramBankSize ) ) );
}

CoreCartridge::CoreCartridge( std::vector<uint8_t>&& rom_ ) : rom( std::move( rom_ ) ) {
    logDebug( "CoreCartridge constructor" );

    logDebug( std::format( "Read cartridgeType byte: {}", toHex( rom[addr::cartridgeType] ) ) );

    const auto romSizeByte = rom[addr::romSize];
    logDebug( std::format( "Read ROM size byte: {}", toHex( romSizeByte ) ) );
    initRom( static_cast<CoreCartridge::RomSizeByte>( romSizeByte ) );

    const auto ramSizeByte = rom[addr::ramSize];
    logDebug( std::format( "Read RAM size byte: {}", toHex( ramSizeByte ) ) );
    initRam( static_cast<CoreCartridge::RamSizeByte>( ramSizeByte ) );
};


bool CoreCartridge::checkCopyRightHeader( const uint16_t bankNumber ) const {
    if( getRomBankCount() < bankNumber ) {
        logWarning( 0, "Bank number out of range." );
        return false;
    }

    const auto isLogoEqual =
            std::equal( romBanks[bankNumber].begin() + addr::logoStart,
                        romBanks[bankNumber].begin() + addr::logoEnd, std::begin( nintendoCopyrightHeader ) );

    [[maybe_unused]] const auto checkMBC1M = bankNumber != 0;
    if( isLogoEqual ) {
        logDebug( std::format( "Nintendo copyright header matches! This is nintendo {}cartridge.",
                               checkMBC1M ? "MBC1M " : "" ) );
    } else {
        logDebug( std::format( "Nintendo copyright header does not match. This is not nintendo {}cartridge.",
                               checkMBC1M ? "MBC1M " : "" ) );
    }

    return isLogoEqual;
}
