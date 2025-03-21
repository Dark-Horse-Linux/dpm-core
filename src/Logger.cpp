// Logger.cpp
#include "Logger.hpp"

// Global logger instance
Logger g_logger;

Logger::Logger()
    : log_level(DPMDefaults::LOG_LEVEL),
      log_to_file(DPMDefaults::write_to_log),
      log_file(DPMDefaults::LOG_FILE)
{
}

Logger::~Logger()
{
}

void Logger::setLogFile(const std::string& new_log_file)
{
    log_file = new_log_file;

    // If logging to file is enabled, ensure the log directory exists and is writable
    if (log_to_file) {
        std::filesystem::path log_path(log_file);
        std::filesystem::path log_dir = log_path.parent_path();

        // Check if the directory exists, create if not
        if (!log_dir.empty() && !std::filesystem::exists(log_dir)) {
            try {
                if (!std::filesystem::create_directories(log_dir)) {
                    std::cerr << "FATAL: Failed to create log directory: " << log_dir.string() << std::endl;
                    exit(1);
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "FATAL: Error creating log directory: " << e.what() << std::endl;
                exit(1);
            }
        }

        // Verify we can write to the log file
        try {
            std::ofstream test_log_file(log_file, std::ios::app);
            if (!test_log_file.is_open()) {
                std::cerr << "FATAL: Cannot open log file for writing: " << log_file << std::endl;
                exit(1);
            }
            test_log_file.close();
        } catch (const std::exception& e) {
            std::cerr << "FATAL: Error validating log file access: " << e.what() << std::endl;
            exit(1);
        }
    }
}

void Logger::setWriteToLog(bool new_write_to_log)
{
    log_to_file = new_write_to_log;

    // If logging was just enabled, validate the log file
    if (log_to_file) {
        setLogFile(log_file);
    }
}

void Logger::setLogLevel(LoggingLevels new_log_level)
{
    log_level = new_log_level;
}

std::string Logger::LogLevelToString(LoggingLevels level)
{
    switch (level) {
        case LoggingLevels::FATAL:
            return "FATAL";
        case LoggingLevels::ERROR:
            return "ERROR";
        case LoggingLevels::WARN:
            return "WARN";
        case LoggingLevels::INFO:
            return "INFO";
        case LoggingLevels::DEBUG:
            return "DEBUG";
        default:
            return "UNKNOWN";
    }
}

LoggingLevels Logger::stringToLogLevel(const std::string& level_str, LoggingLevels default_level)
{
    if (level_str == "FATAL") {
        return LoggingLevels::FATAL;
    } else if (level_str == "ERROR") {
        return LoggingLevels::ERROR;
    } else if (level_str == "WARN") {
        return LoggingLevels::WARN;
    } else if (level_str == "INFO") {
        return LoggingLevels::INFO;
    } else if (level_str == "DEBUG") {
        return LoggingLevels::DEBUG;
    }

    // Return default if no match
    return default_level;
}

void Logger::log(LoggingLevels message_level, const std::string& message)
{
    // Only process if the message level is less than or equal to the configured level
    if (message_level <= log_level) {
        // Convert log level to string
        std::string level_str = LogLevelToString(message_level);

        // Console output without timestamp
        if (message_level == LoggingLevels::FATAL ||
            message_level == LoggingLevels::ERROR ||
            message_level == LoggingLevels::WARN) {
            // Send to stderr
            std::cerr << level_str << ": " << message << std::endl;
        } else {
            // Send to stdout
            std::cout << message << std::endl;
        }

        // Write to log file if enabled (with timestamp)
        if (log_to_file) {
            try {
                // Get current time for timestamp (only for log file)
                std::time_t now = std::time(nullptr);
                char timestamp[32];
                std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

                // Full formatted message with timestamp for log file
                std::string formatted_message = std::string(timestamp) + " [" + level_str + "] " + message;

                std::ofstream log_stream(log_file, std::ios::app);
                if (log_stream.is_open()) {
                    log_stream << formatted_message << std::endl;
                    log_stream.close();
                } else {
                    std::cerr << "Failed to write to log file: " << log_file << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error writing to log file: " << e.what() << std::endl;
            }
        }
    }
}