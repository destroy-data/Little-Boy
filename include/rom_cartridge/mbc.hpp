#pragma once
#include <cstdint>

class MBCController {
protected:
    uint8_t currentRomBank = 1;
    uint8_t currentRamBank = 0;
    bool ramEnabled = false;

public:
    // Check if given write changes RAM/ROM banks and set them accordingly
    virtual void handleBankingWrite( uint16_t address, uint8_t value ) = 0;

    struct PhysicalAddress {
        uint8_t bank;
        uint16_t address;
        bool isRam;
    };

    PhysicalAddress translateAddress( uint16_t cpuAddress );
};
