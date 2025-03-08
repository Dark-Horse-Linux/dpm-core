/**
 * @file Logger.hpp
 * @brief Logging system for the DPM utility
 * 
 * Defines the Logger class which provides a centralized logging facility
 * for the DPM utility. Supports multiple log levels, console output, and
 * optional file logging with timestamps.
 * 
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 * 
 * Part of the Dark Horse Linux Package Manager (DPM)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * 
 * For bug reports or contributions, please contact the dhlp-contributors
 * mailing list at: https://lists.darkhorselinux.org/mailman/listinfo/dhlp-contributors
 */

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

/**
 * @class Logger
 * @brief Provides logging capabilities for the DPM system
 * 
 * Implements a configurable logging system that can write messages to
 * both console and file outputs. Supports different log levels to
 * control verbosity and includes automatic timestamp generation for
 * log file entries.
 */
class Logger {
public:
    /**
     * @brief Constructor
     * 
     * Initializes a new Logger instance with default settings from DPMDefaults.
     */
    Logger();

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Logs a message with the specified log level
     * 
     * Writes a log message to the console and optionally to a log file
     * if the message level is less than or equal to the configured log level.
     * Messages with levels FATAL, ERROR, or WARN are written to stderr,
     * while others go to stdout. File logging includes timestamps.
     * 
     * @param log_level The severity level of the message
     * @param message The message to log
     */
    void log(LoggingLevels log_level, const std::string& message);

    /**
     * @brief Sets the log file path
     * 
     * Changes the file path where log messages are written when file
     * logging is enabled. Ensures the log directory exists and is writable.
     * 
     * @param log_file The path to the log file
     */
    void setLogFile(const std::string& log_file);

    /**
     * @brief Enables or disables file logging
     * 
     * Controls whether log messages are written to a file in addition
     * to console output. If enabled, validates that the log file is writable.
     * 
     * @param write_to_log true to enable file logging, false to disable
     */
    void setWriteToLog(bool write_to_log);

    /**
     * @brief Sets the log level threshold
     * 
     * Sets the maximum log level that will be processed. Messages with
     * a level higher than this threshold will be ignored.
     * 
     * @param log_level The new log level threshold
     */
    void setLogLevel(LoggingLevels log_level);

    /**
     * @brief Converts a log level string to the corresponding enum value
     * 
     * Parses a string representation of a log level (e.g., "INFO", "DEBUG")
     * and returns the corresponding LoggingLevels enum value.
     * 
     * @param level_str The string representation of the log level
     * @param default_level The default level to return if parsing fails
     * @return The corresponding LoggingLevels enum value
     */
    static LoggingLevels stringToLogLevel(const std::string& level_str, LoggingLevels default_level = LoggingLevels::INFO);

    /**
     * @brief Converts a LoggingLevels enum value to its string representation
     * 
     * Returns the string representation of a log level (e.g., "INFO", "DEBUG")
     * for the given LoggingLevels enum value.
     * 
     * @param level The LoggingLevels enum value
     * @return The string representation of the log level
     */
    static std::string LogLevelToString(LoggingLevels level);

private:
    /**
     * @brief The current log level threshold
     */
    LoggingLevels log_level;

    /**
     * @brief Whether to write log messages to a file
     */
    bool log_to_file;

    /**
     * @brief The path to the log file
     */
    std::string log_file;
};

/**
 * @brief Global logger instance
 * 
 * Provides a single instance of the Logger that can be accessed
 * from anywhere in the application.
 */
extern Logger g_logger;