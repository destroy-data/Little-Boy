#include "core/ppu.hpp"
#include <utility>

void CorePpu::BackgroundFetcher::tick() {
    const uint8_t lcdc = mem.read( addr::lcdControl );
    const bool bgWinEnabled = lcdc & 0x1;
    const uint8_t scrollY = mem.read( addr::bgScrollY );
    const uint8_t ly = mem.read( addr::lcdY );
    //TODO handle transitioning to window mid-tile
    switch( state ) {
        using enum FetcherState_t;
    case FETCH_TILE:
        if( !ticksInCurrentState )
            ticksInCurrentState++;
        else {
            if( !bgWinEnabled )
                return;

            const uint8_t winX = mem.read( addr::winX );
            const uint8_t winY = mem.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile =
                    winEnabled && ( currentTileX * 8 >= static_cast<uint8_t>( winX - 7 ) );
            const bool useSecondMap =
                    ( windowTile && lcdc & ( 1 << 6 ) ) || ( !windowTile && lcdc & ( 1 << 3 ) );

            unsigned tileY;
            if( windowTile ) {
                tileY = ( ( ly - winY ) / 8 ) % 8;
            } else {
                tileY = ( ( scrollY + ly ) / 8 ) % 8;
            }
            tileId = ppu.tilemap[useSecondMap][tileY * 32 + currentTileX];
            state = FETCH_DATA_LOW;
            ticksInCurrentState = 0;
        }
        break;
    case FETCH_DATA_LOW:
        if( !ticksInCurrentState )
            ticksInCurrentState++;
        else {
            const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
            const int tileAddress =                      // 0 is start of VRAM
                    base8000Addr ? tileSize * tileId
                                 : 0x1000 + tileSize * static_cast<int8_t>( tileId );

            const uint8_t winX = mem.read( addr::winX );
            const uint8_t winY = mem.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile =
                    winEnabled && ( winY <= ly ) && ( currentTileX * 8 >= winX - 7 );

            int row;
            if( windowTile )
                row = ( ly - winY ) % 8;
            else
                row = ( ly + scrollY ) % 8;
            tileDataLow = mem.videoRam[uint16_t( tileAddress + row * 2 )];
            state = FETCH_DATA_HIGH;
            ticksInCurrentState = 0;
        }
        break;
    case FETCH_DATA_HIGH:
        if( !ticksInCurrentState )
            ticksInCurrentState++;
        else {
            const bool base8000Addr = lcdc & ( 1 << 4 ); // For background and window tiles
            const int tileAddress =                      // 0 is start of VRAM
                    base8000Addr ? tileSize * tileId
                                 : 0x1000 + tileSize * static_cast<int8_t>( tileId );

            const uint8_t winX = mem.read( addr::winX );
            const uint8_t winY = mem.read( addr::winY );
            const bool winEnabled = ( lcdc & ( 1 << 5 ) );
            const bool windowTile =
                    winEnabled && ( winY <= ly ) && ( currentTileX * 8 >= winX - 7 );

            int row;
            if( windowTile )
                row = ( ly - winY ) % 8;
            else
                row = ( ly + scrollY ) % 8;
            tileDataHigh = mem.videoRam[uint16_t( tileAddress + row * 2 + 1 )];
            state = PUSH;
            ticksInCurrentState = 0;
        }
        break;
    case PUSH:
        // How tiles are encoded:
        // https://gbdev.io/pandocs/Tile_Data.html#data-format
        if( ppu.state.bgPixelsFifo.empty() ) {
            for( int i = 7; i >= 0; i-- ) {
                if( bgWinEnabled ) {
                    const bool lowerBit = tileDataLow & ( 1 << i );
                    const bool higherBit = tileDataHigh & ( 1 << i );
                    ppu.state.bgPixelsFifo.push(
                            Pixel( static_cast<uint8_t>( lowerBit | ( higherBit << 1 ) ) ) );
                } else
                    ppu.state.bgPixelsFifo.push( Pixel( 0 ) );
            }
            state = FETCH_TILE;
            ticksInCurrentState = 0;
            currentTileX++;
        }
        break;
    default:
        std::unreachable();
    }
}

void CorePpu::BackgroundFetcher::reset() {
    state = FetcherState_t::FETCH_TILE;
    ticksInCurrentState = 0;
    currentTileX = 0;
}

//--------------------------------------------------
void CorePpu::SpriteFetcher::tick() {
    const uint8_t lcdc = mem.read( addr::lcdControl );
    const bool bgWinEnabled = lcdc & 0x1;
    const bool objEnabled = lcdc & ( 1 << 1 );

    const uint8_t scrollX = mem.read( addr::bgScrollX );
    const uint8_t scrollY = mem.read( addr::bgScrollY );
    const uint8_t ly = mem.read( addr::lcdY );
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

void CorePpu::SpriteFetcher::reset() {
    state = FetcherState_t::FETCH_TILE;
    ticksInCurrentState = 0;
}
