#pragma once
#include "core/bus.hpp"
#include <cstdint>


class Fetcher {
protected:
    enum class FetcherState_t { FETCH_TILE, FETCH_DATA_LOW, FETCH_DATA_HIGH, PUSH };
    FetcherState_t state = FetcherState_t::FETCH_TILE;
    uint_fast8_t ticksInCurrentState;
    uint8_t tileId;
    uint8_t tileDataLow;
    uint8_t tileDataHigh;
    IBus& bus;
    PixelFifo& pixelFifo;

public:
    Fetcher( IBus& bus_, PixelFifo& fifo ) : bus( bus_ ), pixelFifo( fifo ) {
    }
    virtual void tick()  = 0;
    virtual void reset() = 0;
};

class BackgroundFetcher final : public Fetcher {
public:
    uint8_t currentTileX;
    BackgroundFetcher( IBus& bus_, PixelFifo& fifo ) : Fetcher( bus_, fifo ) {
    }
    void tick() override;
    void reset() override;
};

class SpriteFetcher final : public Fetcher {
public:
    SpriteFetcher( IBus& bus_, PixelFifo& fifo ) : Fetcher( bus_, fifo ) {
    }
    void tick() override;
    void reset() override;
};
