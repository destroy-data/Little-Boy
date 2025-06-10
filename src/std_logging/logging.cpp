#include "core/logging.hpp"
#include <filesystem>
#include <print>
#include <stacktrace>

namespace ansi {
// Text formatting
constexpr auto reset     = "\033[0m";
constexpr auto bold      = "\033[1m";
constexpr auto dim       = "\033[2m";
constexpr auto italic    = "\033[3m";
constexpr auto underline = "\033[4m";
constexpr auto blink     = "\033[5m";
constexpr auto reverse   = "\033[7m";
constexpr auto hidden    = "\033[8m";

// Foreground colors
constexpr auto black     = "\033[30m";
constexpr auto red       = "\033[31m";
constexpr auto green     = "\033[32m";
constexpr auto yellow    = "\033[33m";
constexpr auto blue      = "\033[34m";
constexpr auto magenta   = "\033[35m";
constexpr auto cyan      = "\033[36m";
constexpr auto white     = "\033[37m";
constexpr auto defaultFg = "\033[39m";
constexpr auto defaultBg = "\033[49m";

// Bright foreground colors
constexpr auto brightBlack   = "\033[90m";
constexpr auto brightRed     = "\033[91m";
constexpr auto brightGreen   = "\033[92m";
constexpr auto brightYellow  = "\033[93m";
constexpr auto brightBlue    = "\033[94m";
constexpr auto brightMagenta = "\033[95m";
constexpr auto brightCyan    = "\033[96m";
constexpr auto brightWhite   = "\033[97m";

// Background colors
constexpr auto bgBlack   = "\033[40m";
constexpr auto bgRed     = "\033[41m";
constexpr auto bgGreen   = "\033[42m";
constexpr auto bgYellow  = "\033[43m";
constexpr auto bgBlue    = "\033[44m";
constexpr auto bgMagenta = "\033[45m";
constexpr auto bgCyan    = "\033[46m";
constexpr auto bgWhite   = "\033[47m";

// Bright background colors
constexpr auto bgBrightBlack   = "\033[100m";
constexpr auto bgBrightRed     = "\033[101m";
constexpr auto bgBrightGreen   = "\033[102m";
constexpr auto bgBrightYellow  = "\033[103m";
constexpr auto bgBrightBlue    = "\033[104m";
constexpr auto bgBrightMagenta = "\033[105m";
constexpr auto bgBrightCyan    = "\033[106m";
constexpr auto bgBrightWhite   = "\033[107m";
} // namespace ansi

using namespace ansi;

constexpr std::string_view source_locations[] = { "include/", "src/", "test/" };

std::string_view getRelevantPartOfPath( const std::string_view filePath ) {
    for( const auto loc: source_locations ) {
        const auto pos = filePath.rfind( loc );
        if( pos != std::string::npos ) {
            return filePath.substr( pos );
        }
    }
    // fallback
    return filePath;
}

void log( const int errorCode, const ErrorSeverity severity, const std::string& message,
          const std::string_view filePath, const int line ) {
    std::string_view severityStr;
    std::string_view format;
    std::string_view color;
    switch( severity ) {
    case ErrorSeverity::Debug:
        severityStr = "[DEBUG]";
        color       = green;
        break;
    case ErrorSeverity::Info:
        severityStr = "[INFO]";
        color       = brightGreen;
        break;
    case ErrorSeverity::Warning:
        severityStr = "[WARNING]";
        color       = magenta;
        break;
    case ErrorSeverity::Error:
        severityStr = "[ERROR]";
        color       = red;
        break;
    case ErrorSeverity::Fatal:
        severityStr = "[FATAL]";
        format      = bold;
        color       = brightYellow;
        break;
    default:
        severityStr = "[UNKNOWN]";
        break;
    }

    std::print( "{}{}{:<9}#{} code {:04d}: {} ({}:{})\n", format, color, severityStr, reset, errorCode,
                message, getRelevantPartOfPath( filePath ), line );
}

void logStacktrace() {
    std::println( "{}{}STACKTRACE{}", bold, brightYellow, reset );
    const auto stacktrace = std::stacktrace::current();
    for( size_t i = 0; const auto& entry: stacktrace ) {
        const std::string filePath    = entry.source_file();
        const std::string description = entry.description();
        std::println( "{}{:4d}#{} {} at {}:{}", green, i++, reset, description,
                      getRelevantPartOfPath( filePath ), entry.source_line() );

        if( description == "main" )
            break;
    }
}

void logSeparator() {
    std::println( "{:<9}# --------------------------------------------------", "" );
}
