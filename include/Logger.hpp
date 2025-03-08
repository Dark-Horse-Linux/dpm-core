#pragma once

#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <cstdlib>

#include "LoggingLevels.hpp"
#include "DPMDefaults.hpp"

class Logger {
public:
    // constructor
    Logger();

    // destructor
    ~Logger();

    // Log method that accepts a string
    void log(LoggingLevels log_level, const std::string& message);

    // Configuration setters
    void setLogFile(const std::string& log_file);
    void setWriteToLog(bool write_to_log);
    void setLogLevel(LoggingLevels log_level);

    // String to LoggingLevels conversion
    static LoggingLevels stringToLogLevel(const std::string& level_str, LoggingLevels default_level = LoggingLevels::INFO);

    // Convert LoggingLevels enum to string
    static std::string LogLevelToString(LoggingLevels level);

private:
    // the logging level to stay initialized to
    LoggingLevels log_level;

    // whether or not to log to file
    bool log_to_file;

    // log file path
    std::string log_file;
};

// Global logger instance
extern Logger g_logger;
