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
    InvalidOperationType         = 400,
    InvalidOperandType           = 401,
    InvalidOperand               = 402,
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

enum class ErrorSeverity : uint8_t {
    Debug,   // Details
    Info,    // What happens
    Warning, // Unexpected situation and/or ignored can cause error in the future
    Error,   // Something went wrong
    Fatal    // Something went very wrong, cannot continue execution
};

void log( const int errorCode, const ErrorSeverity severity, const std::string& message,
          const std::string_view filePath, const int line );

void logStacktrace();

#define logDebug( message ) log( ErrorCode::NoError, ErrorSeverity::Debug, message, __FILE__, __LINE__ )

#define logInfo( message ) log( ErrorCode::NoError, ErrorSeverity::Info, message, __FILE__, __LINE__ )

#define logWarning( code, message ) log( code, ErrorSeverity::Warning, message, __FILE__, __LINE__ )

#define logError( code, message ) log( code, ErrorSeverity::Error, message, __FILE__, __LINE__ )

#define logFatal( code, message ) log( code, ErrorSeverity::Fatal, message, __FILE__, __LINE__ )

template<std::integral T>
inline std::string toHex( T number ) {
    const auto padding = sizeof( T ) * 2;
    return std::format( "0x{:0{}X}", number, padding );
}
