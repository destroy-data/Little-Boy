#include "core/logging.hpp"
#include <filesystem>
#include <print>

void log( int errorCode, ErrorSeverity severity, std::string message, std::string file, int line ) {
    std::string_view severityStr;
    switch( severity ) {
    case ErrorSeverity::Debug:
        severityStr = "DEBUG";
        break;
    case ErrorSeverity::Info:
        severityStr = "INFO";
        break;
    case ErrorSeverity::Warning:
        severityStr = "WARNING";
        break;
    case ErrorSeverity::Error:
        severityStr = "ERROR";
        break;
    case ErrorSeverity::Fatal:
        severityStr = "FATAL";
        break;
    default:
        severityStr = "UNKNOWN";
        break;
    }

    std::filesystem::path filePath( file );
    std::string filename = filePath.filename().string();
    std::print( "[{}] code {:04d}: {} ({}:{})\n", severityStr, errorCode, message, filename, line );
}
