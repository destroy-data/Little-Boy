#include "cartridge_impls/cartridge.hpp"
#include "core/cartridge.hpp"
#include "core/logging.hpp"
#include <cstdint>
#include <format>


MBC3Cartridge::MBC3Cartridge( std::vector<uint8_t>&& rom_, bool hasTimer )
    : CoreCartridge( std::move( rom_ ) ) {

    logDebug( "MBC3Cartridge with" + std::string( hasTimer ? " Timer" : "out Timer" ) + " constructor." );
    if( hasTimer ) {
        rtc = std::make_unique<RTC>();
    }

    if( getRomSize() > RomSize::_2MiB ) {
        logError( 0, "ROM size greater than 2 MiB is not supported by MBC3 cartridge." );
        return;
    }

    if( getRamSize() > RamSize::_32KiB ) {
        logError( 0, "RAM size greater than 32 KiB is not supported by MBC3 cartridge." );
        return;
    }
}

uint8_t MBC3Cartridge::readRom( const uint16_t address, bool isPrimaryRom ) const {
    const auto romBankIndex = isPrimaryRom ? 0u : romSelectRegister;
    const auto bankOffset   = static_cast<uint16_t>(
            address - ( isPrimaryRom ? romStartAddress : ( romStartAddress + romBankSize ) ) );
    const auto returnValue = romBanks[romBankIndex][bankOffset];

    logInfo( std::format( "Read value {} from ROM bank {} at offset {}", toHex( returnValue ), romBankIndex,
                          toHex( bankOffset ) ) );
    return returnValue;
}

uint8_t MBC3Cartridge::read( const uint16_t address ) {
    logDebug( std::format( "Trying to read at address {}", toHex( address ) ) );

    if( isInPrimaryRomRange( address ) ) {
        return readRom( address, true );
    }

    if( isInSecondaryRomRange( address ) ) {
        return readRom( address, false );
    }

    if( isInRamRange( address ) ) {
        if( isValueInRamBankRange( ramBankOrRtcSelectRegister ) ) {
            const auto returnValue = ramBanks[ramBankOrRtcSelectRegister][address - ramStartAddress];
            logInfo( std::format( "Read value {} from RAM bank {} at address {}", toHex( returnValue ),
                                  ramBankOrRtcSelectRegister, toHex( address ) ) );
            return returnValue;
        }

        if( isValueInRtcRegisterRange( ramBankOrRtcSelectRegister ) ) {
            if( ! rtc ) {
                logWarning( 0, std::format( "MBC3 cartridge without timer tried to read from RTC register {}.",
                                            toHex( ramBankOrRtcSelectRegister ) ) );
                return invalidReadValue;
            }

            uint8_t returnValue;
            switch( static_cast<RTC::Register>( ramBankOrRtcSelectRegister ) ) {
            case RTC::Register::Seconds:
                returnValue = rtc->getLatchTimeValue<std::chrono::seconds>();
                break;
            case RTC::Register::Minutes:
                returnValue = rtc->getLatchTimeValue<std::chrono::minutes>();
                break;
            case RTC::Register::Hours:
                returnValue = rtc->getLatchTimeValue<std::chrono::hours>();
                break;
            case RTC::Register::DayLow:
                returnValue = rtc->getLatchTimeValue<std::chrono::days>();
                break;
            case RTC::Register::DayHigh:
                returnValue = rtc->getLatchTimeValue<std::chrono::days>( true );
                break;
            default:
                [[unlikely]] std::unreachable();
            }

            logInfo( std::format( "Read from RTC register {}: {}", toHex( ramBankOrRtcSelectRegister ),
                                  toHex( returnValue ) ) );
            return returnValue;
        }

        logWarning( 0, std::format( "RAM bank {} is not valid. Returning {}",
                                    toHex( ramBankOrRtcSelectRegister ), toHex( invalidReadValue ) ) );
        return invalidReadValue;
    }

    logWarning( 0, std::format( "Address {} is not in RAM nor ROM range. Returning {}", toHex( address ),
                                toHex( invalidReadValue ) ) );
    return invalidReadValue;
}

void MBC3Cartridge::write( const uint16_t address, const uint8_t value ) {

    if( isInRamOrRtcEnableRange( address ) ) {
        ramAndRtcEnabled = value == ramEnableValue;
        logInfo( std::format( "RAM/RTC access {}abled.", ramAndRtcEnabled ? "en" : "dis" ) );
        return;
    }

    if( isInRomBankSelectRange( address ) ) {
        if( ! isValueInRomBankRange( value ) ) {
            logWarning( 0,
                        std::format( "Value {} outside of valid range. ROM bank select register not updated.",
                                     toHex( value ) ) );
            return;
        }

        romSelectRegister =
                value == 0 ? 1 : value; // ROM bank 0 is not allowed, so we set it to 1 if 0 is written
        logInfo( std::format( "Selected ROM bank set to: {}", toHex( romSelectRegister ) ) );
        return;
    }

    if( isInRamOrRTCSelectRange( address ) ) {
        if( ! ( isValueInRamBankRange( value ) || ( rtc && isValueInRtcRegisterRange( value ) ) ) ) {
            logWarning( 0, std::format( "Value {} outside of valid range. Select register not updated.",
                                        toHex( value ) ) );
            return;
        }
        ramBankOrRtcSelectRegister = value;
        logInfo( std::format( "Selected RAM/RTC register set to: {}", toHex( ramBankOrRtcSelectRegister ) ) );
        return;
    }

    if( isInRtcLatchRange( address ) ) {
        if( ! rtc ) {
            logWarning( 0, "MBC3 cartridge without timer tried to latch RTC." );
            return;
        }

        if( lastLatchWriteValue == 0 && value == 1 ) {
            rtc->latchTime();
            logInfo( "RTC time latched." );
        }
        lastLatchWriteValue = value;
        logInfo( std::format( "RTC latch register set to: {}", toHex( value ) ) );
        return;
    }

    if( isInRamRange( address ) ) {
        if( ! ramAndRtcEnabled ) {
            logWarning( 0, "RAM/Timer is not enabled. Cannot write to RAM." );
            return;
        }

        if( isValueInRamBankRange( ramBankOrRtcSelectRegister ) ) {
            const uint16_t offset                        = address - ramStartAddress;
            ramBanks[ramBankOrRtcSelectRegister][offset] = value;
            logInfo( std::format( "Wrote value {} to RAM bank {} at address {}", toHex( value ),
                                  ramBankOrRtcSelectRegister, toHex( offset ) ) );
            return;
        }

        if( isValueInRtcRegisterRange( ramBankOrRtcSelectRegister ) ) {
            if( ! rtc ) {
                logWarning( 0, "MBC3 cartridge without timer tried to write to RTC register." );
                return;
            }

            switch( static_cast<RTC::Register>( ramBankOrRtcSelectRegister ) ) {
            case RTC::Register::Seconds:
                if( value < 60 ) {
                    rtc->updateTime( std::chrono::seconds( value ) );
                } else {
                    logWarning( 0, std::format( "Invalid seconds value: {}. Must be less than 60.", value ) );
                }
                break;
            case RTC::Register::Minutes:
                if( value < 60 ) {
                    rtc->updateTime( std::chrono::minutes( value ) );
                } else {
                    logWarning( 0, std::format( "Invalid minutes value: {}. Must be less than 60.", value ) );
                }
                break;
            case RTC::Register::Hours:
                if( value < 24 ) {
                    rtc->updateTime( std::chrono::hours( value ) );
                } else {
                    logWarning( 0, std::format( "Invalid hours value: {}. Must be less than 24.", value ) );
                }
                break;
            case RTC::Register::DayLow:
                rtc->updateDay( value, false );
                break;
            case RTC::Register::DayHigh:
                rtc->updateRegisters( value );
                break;
            default:
                std::unreachable();
            }
            return;
        }

        logWarning( 0, std::format( "ramBankOrRtcSelectRegister {} is not valid. Cannot write to it.",
                                    toHex( ramBankOrRtcSelectRegister ) ) );
        return;
    }
}

template<typename T>
void MBC3Cartridge::RTC::updateTime( const T newTime ) {
    if( ! isHalted ) {
        logWarning( 0, "RTC is not halted. Cannot update time." );
        return;
    }

    if constexpr( std::is_same_v<T, std::chrono::seconds> ) {
        logDebug(
                std::format( "Update seconds from {} to {}", latchedTime.seconds.count(), newTime.count() ) );
        latchedTime.seconds = newTime;
        rtcTime.seconds     = newTime;
        return;
    } else if constexpr( std::is_same_v<T, std::chrono::minutes> ) {
        logDebug(
                std::format( "Update minutes from {} to {}", latchedTime.minutes.count(), newTime.count() ) );
        latchedTime.minutes = newTime;
        rtcTime.minutes     = newTime;
        return;
    } else if constexpr( std::is_same_v<T, std::chrono::hours> ) {
        logDebug( std::format( "Update hours from {} to {}", latchedTime.hours.count(), newTime.count() ) );
        latchedTime.hours = newTime;
        rtcTime.hours     = newTime;
        return;
    }

    logWarning( 0, std::format( "Unsupported updateTime type: {}.", typeid( T ).name() ) );
}

void MBC3Cartridge::RTC::updateDay( const uint8_t newDay, const bool isDayHighRegister ) {
    if( ! isHalted ) {
        logWarning( 0, "RTC is not halted. Cannot update day." );
        return;
    }

    auto updateDayNumber = [newDay, isDayHighRegister]( timeRegister& time ) {
        auto dayNumber = time.days.count();
        if( ! isDayHighRegister ) {
            dayNumber = ( dayNumber & ~0xFF ) | newDay;
        } else {
            // if dayHighBit is set then do OR on it - set it
            // else do AND NOT on it - clear
            if( newDay & bitMasks::dayHighDayCounter ) {
                dayNumber |= 0x100;
            } else {
                dayNumber &= ~0x100;
            }
        }
        time.days = std::chrono::days( dayNumber );
    };

    updateDayNumber( rtcTime );
    updateDayNumber( latchedTime );
}

void MBC3Cartridge::RTC::updateRegisters( const uint8_t value ) {
    const auto newIsHaltedValue = ( value & bitMasks::dayHighHalt ) != 0;
    if( newIsHaltedValue && ! isHalted ) {
        logDebug( "RTC just halted." );
        captureRtcTime();
    }
    if( ! newIsHaltedValue && isHalted ) {
        logDebug( "RTC just resumed." );
        referenceTime = SteadyClock::now();
    }

    if( isHalted && newIsHaltedValue ) {
        // clear overflow bit only if RTC was already halted and is not currently resumed from the halt
        if( const auto newHasOverflowed = ( value & bitMasks::dayHighOverflow ) != 0; ! newHasOverflowed ) {
            logInfo( "RTC overflow bit cleared." );
            hasOverflowed = newHasOverflowed;
        }
        updateDay( value, true );
    } else {
        logInfo( "RTC is being resummed or halted. Dont modify any register" );
    }
    isHalted = newIsHaltedValue;
}

template<typename T>
uint8_t MBC3Cartridge::RTC::getLatchTimeValue( const bool isDayHighRegister ) const {
    if constexpr( std::is_same_v<T, std::chrono::days> ) {
        if( isDayHighRegister ) {
            return static_cast<uint8_t>(
                    ( isHalted ? bitMasks::dayHighHalt : 0 ) |
                    ( hasOverflowed ? bitMasks::dayHighOverflow : 0 ) |
                    ( ( ( latchedTime.days.count() >> 8 ) & bitMasks::dayHighDayCounter ) ? 1 : 0 ) );
        }
        return static_cast<uint8_t>( latchedTime.days.count() & 0xFF );
    } else if constexpr( std::is_same_v<T, std::chrono::hours> ) {
        return static_cast<uint8_t>( latchedTime.hours.count() );
    } else if constexpr( std::is_same_v<T, std::chrono::minutes> ) {
        return static_cast<uint8_t>( latchedTime.minutes.count() );
    } else if constexpr( std::is_same_v<T, std::chrono::seconds> ) {
        return static_cast<uint8_t>( latchedTime.seconds.count() );
    }
}

void MBC3Cartridge::RTC::checkSetOverflow() {
    if( const auto days = rtcTime.days.count(); days > rtcDayCounterMax ) {
        logDebug( "RTC day counter overflowed." );
        hasOverflowed = true;

        rtcTime.days = std::chrono::days( days % ( rtcDayCounterMax + 1 ) );
    }
}

void MBC3Cartridge::RTC::captureRtcTime() {
    if( isHalted ) {
        logDebug( "RTC is halted. Capture not needed." );
        return;
    }

    const auto currentTime = SteadyClock::now();
    auto elapsedTime       = currentTime - referenceTime;
    elapsedTime += rtcTime.days + rtcTime.hours + rtcTime.minutes + rtcTime.seconds;

    rtcTime.days = std::chrono::duration_cast<std::chrono::days>( elapsedTime );
    elapsedTime -= rtcTime.days;
    rtcTime.hours = std::chrono::duration_cast<std::chrono::hours>( elapsedTime );
    elapsedTime -= rtcTime.hours;
    rtcTime.minutes = std::chrono::duration_cast<std::chrono::minutes>( elapsedTime );
    elapsedTime -= rtcTime.minutes;
    rtcTime.seconds = std::chrono::duration_cast<std::chrono::seconds>( elapsedTime );

    checkSetOverflow();
    referenceTime = currentTime;
}
