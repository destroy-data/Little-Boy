#pragma once
#include <cstdint>
#include <format>
#include <string>

namespace ErrorCode {
// Enum is deliberately leaked to the namespace
// The namespace is used as an extendable enum, front end can add new codes to it the same way
enum CoreError : int {
    // gb errors (0-199)
    NoError        = 0,
    GenericError   = 1,
    NotImplemented = 2,

    // Memory errors (200-399)

    // CPU errors (400-599)
    unknownMicroCodeExecuted     = 400,
    emptyMicroCodeExecuted       = 402,
    InvalidOperand               = 405,
    CPUHardLocked                = 410,
    StackOverflow                = 420,
    StackUnderflow               = 421,
    CpuTimingImplementationError = 430,

    // Cartridge errors (600-799)
    CartridgeNotFound         = 600,
    InvalidCartridge          = 601,
    UnsupportedCartridgeType  = 602,
    RomHeaderChecksumMismatch = 610,
    RomGlobalCheksumMismatch  = 611,
    BankSwitchingError        = 620,
    SaveDataError             = 630,

    // PPU errors (800-999)
    PPURenderingError   = 800,
    InvalidSpriteAccess = 801,
    VRAMAccessViolation = 802,

    // Timer errors (1000-1199)
    TimerOverflow = 1000,

    // IO errors (1200-1399)
    SerialTransferError = 1200,

    // System errors (1400-1599)
    InterruptError = 1400,

    // Reserved for future core components (1600-2047)

    // Reserved for frontend implementations (2048+)
    FrontendErrorStart = 2048
};
} // namespace ErrorCode

enum class LogLevel : uint8_t {
    Debug     = 0,   // Details
    Info      = 1,   // What happens
    Warning   = 2,   // Unexpected situation and/or ignored can cause error in the future
    Error     = 3,   // Something went wrong
    Fatal     = 4,   // Something went very wrong, cannot continue execution
    LiveDebug = 254, // Logs for actions made during live-debugging
    Off       = 255
};

inline LogLevel gLogLevel = LogLevel::Warning;

inline void setLogLevel( LogLevel level ) {
    gLogLevel = level;
}

inline LogLevel getLogLevel() {
    return gLogLevel;
}

inline bool shouldLog( LogLevel level ) {
    return level >= gLogLevel;
}

void log( const int errorCode, const LogLevel severity, const std::string& message,
          const std::string_view filePath, const int line );

void logStacktrace();
void logSeparator();


#ifdef RELEASE

#define logDebug( message ) (void)0
#define logInfo( message ) (void)0
#define logWarning( code, message ) (void)0

#else

#define logDebug( message )                                                                                   \
    do {                                                                                                      \
        if( shouldLog( LogLevel::Debug ) )                                                                    \
            log( ErrorCode::NoError, LogLevel::Debug, message, __FILE__, __LINE__ );                          \
    } while( 0 )

#define logInfo( message )                                                                                    \
    do {                                                                                                      \
        if( shouldLog( LogLevel::Info ) )                                                                     \
            log( ErrorCode::NoError, LogLevel::Info, message, __FILE__, __LINE__ );                           \
    } while( 0 )

#define logWarning( code, message )                                                                           \
    do {                                                                                                      \
        if( shouldLog( LogLevel::Warning ) )                                                                  \
            log( code, LogLevel::Warning, message, __FILE__, __LINE__ );                                      \
    } while( 0 )

#endif

#define logError( code, message )                                                                             \
    do {                                                                                                      \
        if( shouldLog( LogLevel::Error ) )                                                                    \
            log( code, LogLevel::Error, message, __FILE__, __LINE__ );                                        \
    } while( 0 )

#define logFatal( code, message )                                                                             \
    do {                                                                                                      \
        if( shouldLog( LogLevel::Fatal ) )                                                                    \
            log( code, LogLevel::Fatal, message, __FILE__, __LINE__ );                                        \
    } while( 0 )

#define logLiveDebug( message )                                                                               \
    do {                                                                                                      \
        if( shouldLog( LogLevel::LiveDebug ) )                                                                \
            log( 0, LogLevel::LiveDebug, message, __FILE__, __LINE__ );                                       \
    } while( 0 )

template<std::integral T>
inline std::string toHex( T number ) {
    const auto padding = sizeof( T ) * 2;
    return std::format( "0x{:0{}X}", number, padding );
}
