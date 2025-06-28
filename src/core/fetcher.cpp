#include "core/fetcher.hpp"
#include "core/core_constants.hpp"
#include "core/logging.hpp"
#include <utility>

void BackgroundFetcher::tick() {
    const uint8_t lcdc      = bus.read( addr::lcdControl );
    const bool bgWinEnabled = lcdc & 0x1;
    const uint8_t scrollY   = bus.read( addr::bgScrollY );
    const uint8_t ly        = bus.read( addr::lcdY );
    //TODO handle transitioning to window mid-tile
    switch( state ) {
        using enum FetcherState_t;
    case FETCH_TILE:
        if( ! ticksInCurrentState )
            ticksInCurrentState++;
        else {
            if( ! bgWinEnabled )
                return;

            const uint8_t winX    = bus.read( addr::winX );
            const uint8_t winY    = bus.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile =
                    winEnabled && ( winY <= ly ) && ( currentTileX * 8 >= static_cast<uint8_t>( winX - 7 ) );
            const bool useSecondMap =
                    ( windowTile && lcdc & ( 1 << 6 ) ) || ( ! windowTile && lcdc & ( 1 << 3 ) );

            unsigned tileY;
            if( windowTile ) {
                tileY = ( ( ly - winY ) / 8 ) % 32;
            } else {
                tileY = ( ( scrollY + ly ) / 8 ) % 32;
            }
            const auto address = uint16_t( ( useSecondMap ? 0x9C00 : 0x9800 ) + tileY * 32 + currentTileX );
            tileId             = bus.readVram( address );
            logDebug( std::format( "Read tileID <{}> from address <{}>", tileId, toHex( address ) ) );
            state               = FETCH_DATA_LOW;
            ticksInCurrentState = 0;
        }
        break;
    case FETCH_DATA_LOW:
        if( ! ticksInCurrentState )
            ticksInCurrentState++;
        else {
            const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
            const auto tileAddress  = base8000Addr
                                              ? uint16_t( 0x8000 + size::tile * tileId )
                                              : uint16_t( 0x9000 + size::tile * static_cast<int8_t>( tileId ) );

            const uint8_t winX    = bus.read( addr::winX );
            const uint8_t winY    = bus.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile = winEnabled && ( winY <= ly ) && ( currentTileX * 8 >= winX - 7 );

            int row;
            if( windowTile )
                row = ( ly - winY ) % 8;
            else
                row = ( ly + scrollY ) % 8;
            tileDataLow         = bus.readVram( static_cast<uint16_t>( tileAddress + row * 2 ) );
            state               = FETCH_DATA_HIGH;
            ticksInCurrentState = 0;
        }
        break;
    case FETCH_DATA_HIGH:
        if( ! ticksInCurrentState )
            ticksInCurrentState++;
        else {
            const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
            const auto tileAddress  = base8000Addr
                                              ? uint16_t( 0x8000 + size::tile * tileId )
                                              : uint16_t( 0x9000 + size::tile * static_cast<int8_t>( tileId ) );

            const uint8_t winX    = bus.read( addr::winX );
            const uint8_t winY    = bus.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile = winEnabled && ( winY <= ly ) && ( currentTileX * 8 >= winX - 7 );

            int row;
            if( windowTile )
                row = ( ly - winY ) % 8;
            else
                row = ( ly + scrollY ) % 8;
            tileDataHigh        = bus.readVram( static_cast<uint16_t>( tileAddress + row * 2 + 1 ) );
            state               = PUSH;
            ticksInCurrentState = 0;
        }
        break;
    case PUSH:
        // How tiles are encoded:
        // https://gbdev.io/pandocs/Tile_Data.html#data-format
        if( pixelFifo.empty() ) {
            for( int i = 7; i >= 0; i-- ) {
                if( bgWinEnabled ) {
                    const bool lowerBit  = tileDataLow & ( 1 << i );
                    const bool higherBit = tileDataHigh & ( 1 << i );
                    pixelFifo.push( Pixel( static_cast<uint8_t>( lowerBit | ( higherBit << 1 ) ) ) );
                } else
                    pixelFifo.push( Pixel( 0 ) );
            }
            state               = FETCH_TILE;
            ticksInCurrentState = 0;
            currentTileX++;
        }
        break;
    default:
        std::unreachable();
    }
}

void BackgroundFetcher::reset() {
    state               = FetcherState_t::FETCH_TILE;
    ticksInCurrentState = 0;
    currentTileX        = 0;
}

//--------------------------------------------------
void SpriteFetcher::tick() {
    //const uint8_t lcdc = bus.read( addr::lcdControl );
    //const bool objEnabled = lcdc & ( 1 << 1 );

    //const uint8_t scrollX = bus.read( addr::bgScrollX );
    //const uint8_t scrollY = bus.read( addr::bgScrollY );
    //const uint8_t ly = bus.read( addr::lcdY );
    //TODO handle transitioning to window mid-tile
    switch( state ) {
        using enum FetcherState_t;
    case FETCH_TILE: {

    } break;
    case FETCH_DATA_LOW:
        break;
    case FETCH_DATA_HIGH:
        break;
    case PUSH:
        break;
    default:
        std::unreachable();
    }
}

void SpriteFetcher::reset() {
    state               = FetcherState_t::FETCH_TILE;
    ticksInCurrentState = 0;
}
