#include "cartridge_impls/cartridge_factory.hpp"
#include "cartridge_impls/cartridge.hpp"
#include "core/cartridge.hpp"
#include "core/core_constants.hpp"
#include "core/logging.hpp"
#include <format>
#include <memory>
#include <utility>

std::unique_ptr<CoreCartridge> CartridgeFactory::create( std::vector<uint8_t>&& rom ) {
    const auto type = static_cast<CoreCartridge::CartridgeType>( rom[addr::cartridgeType] );

    switch( type ) {
        using enum CoreCartridge::CartridgeType;
    case NoMBC:
        return std::make_unique<NoMBCCartridge>( std::move( rom ) );

    case MBC1:
    case MBC1R:
    case MBC1RB:
        return std::make_unique<MBC1Cartridge>( std::move( rom ) );

    case MBC2:
    case MBC2B:
        return std::make_unique<MBC2Cartridge>( std::move( rom ) );

    case MBC3TB:
    case MBC3TRB:
        return std::make_unique<MBC3Cartridge>( std::move( rom ), true );

    case MBC3:
    case MBC3R:
    case MBC3RB:
        return std::make_unique<MBC3Cartridge>( std::move( rom ) );

    case RR:
        logError( 0, "Cartridge type ROM+RAM is not supported." );
        break;
    case RRB:
        logError( 0, "Cartridge type ROM+RAM+BATTERY is not supported." );
        break;
    default:
        break;
    }
    logError( 0, std::format( "Unknown cartridge type: {}", toHex( std::to_underlying( type ) ) ) );
    return nullptr;
}
