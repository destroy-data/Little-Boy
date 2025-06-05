#include "core/cartridge.hpp"
#include "cartridge_impls/cartridge.hpp"
#include "core/logging.hpp"
#include <format>
#include <type_traits>
#include <utility>

namespace {
unsigned power( unsigned base, unsigned exp ) {
    unsigned result = 1;
    while( exp > 0 ) {
        if( exp % 2 ) {
            result = result * base;
        }
        exp  = exp / 2;
        base = base * base;
    }
    return result;
}
} // namespace

bool CoreCartridge::isValidRomSize( const RomSize size ) {
    switch( size ) {
        using enum RomSize;
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
    logError( 0, std::format( "ROM size unknown. Byte: 0x{:02X}",
                              static_cast<std::underlying_type_t<RomSize>>( size ) ) );
    return false;
}

// Note:
// 2 ^ (RomSize + 1) = number of 16KiB ROM banks
void CoreCartridge::initRom( const RomSize size ) {
    if( !isValidRomSize( size ) ) {
        logError( 0, "Invalid ROM size. Failed to initialize ROM." );
        return;
    }
    romBankCount = power( 2, static_cast<std::underlying_type_t<RomSize>>( size ) + 1 );

    romBanks.clear();
    romBanks.reserve( romBankCount );

    for( unsigned i = 0; i < romBankCount; ++i ) {
        auto offset = i * romBankSize;
        romBanks.emplace_back( rom.data() + offset, romBankSize );
        logDebug( std::format( "Initialized ROM bank {} at offset 0x{:04X} of size 0x{:04X}", i, offset,
                               romBankSize ) );
    }
    logInfo( std::format( "Initialized {} ROM banks of size 0x{:04X} bytes", romBanks.size(), romBankSize ) );
}

bool CoreCartridge::isValidRamSize( const RamSize size ) {
    switch( size ) {
        using enum RamSize;
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
    logError( 0, std::format( "RAM size unknown. Byte: 0x{:02X}",
                              static_cast<std::underlying_type_t<RamSize>>( size ) ) );
    return false;
}

void CoreCartridge::initRam( const CoreCartridge::RamSize size ) {
    if( !isValidRamSize( size ) ) {
        logError( 0, "Invalid RAM size. Failed to initialize RAM." );
        return;
    }

    // Note:
    // RAM consists of 8KiB banks
    switch( size ) {
        using enum RamSize;
    case _0KiB:
        ramBankCount = 0;
        break;
    case _8KiB:
        ramBankCount = 1;
        break;
    case _32KiB:
        ramBankCount = 4;
        break;
    case _64KiB:
        ramBankCount = 8;
        break;
    case _128KiB:
        ramBankCount = 16;
        break;
    default:
        std::unreachable();
    }

    ramBanks = std::vector<std::vector<uint8_t>>( ramBankCount, std::vector<uint8_t>( ramBankSize, 0 ) );
    logInfo( std::format( "Initialized {} RAM banks of size 0x{:04X} bytes", ramBanks.size(), ramBankSize ) );
}

CoreCartridge::CoreCartridge( std::vector<uint8_t>&& rom_ ) : rom( std::move( rom_ ) ) {
    logDebug( "CoreCartridge constructor" );
    logDebug( std::format( "Read ROM size byte: 0x{:02X} bytes", rom[0x148] ) );
    logDebug( std::format( "Read RAM size byte: 0x{:02X} bytes", rom[0x149] ) );

    initRom( static_cast<CoreCartridge::RomSize>( rom[0x148] ) );
    initRam( static_cast<CoreCartridge::RamSize>( rom[0x149] ) );
};

std::unique_ptr<CoreCartridge> CoreCartridge::create( CartridgeType type, std::vector<uint8_t>&& rom ) {
    switch( type ) {
        using enum CartridgeType;
    case NoMBC:
        return std::make_unique<NoMBCCartridge>( std::move( rom ) );

    case RR:
        logError( 0, "Cartridge type ROM+RAM is not supported." );
        break;
    case RRB:
        logError( 0, "Cartridge type ROM+RAM+BATTERY is not supported." );
        break;
    default:
        break;
    }
    logError( 0, std::format( "Unknown cartridge type: 0x{:02X}",
                              static_cast<std::underlying_type_t<CartridgeType>>( type ) ) );
    return nullptr;
}
