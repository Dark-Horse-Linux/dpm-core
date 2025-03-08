/**
 * @file infoFuncs.hpp
 * @brief Header file for the info module support functions
 *
 * Defines functions and enumerations for the info module which provides
 * information about the DPM system, including version, system details,
 * and configuration information.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <cstring>
#include <fstream>
#include <sys/utsname.h>
#include "dpmdk/include/CommonModuleAPI.hpp"

/**
 * @enum Command
 * @brief Enumeration of supported commands for the info module
 */
enum Command {
    CMD_UNKNOWN,    /**< Unknown or unsupported command */
    CMD_HELP,       /**< Display help information */
    CMD_VERSION,    /**< Display version information */
    CMD_SYSTEM,     /**< Display system information */
    CMD_CONFIG      /**< Display configuration information */
};

/**
 * @brief Detects the system architecture
 *
 * Uses the uname system call to determine the current system architecture.
 *
 * @return String representation of the system architecture
 */
std::string detect_architecture();

/**
 * @brief Detects the operating system information
 *
 * Uses the uname system call to determine the OS, and on Linux systems,
 * attempts to get distribution information from /etc/os-release.
 *
 * @return Detailed string of the operating system
 */
std::string detect_os();

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the info module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);

/**
 * @brief Handler for the version command
 *
 * Displays DPM version information, build date, and build time.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_version(int argc, char** argv);

/**
 * @brief Handler for the system command
 *
 * Displays information about the system on which DPM is running,
 * including OS details and architecture.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_system(int argc, char** argv);

/**
 * @brief Handler for the config command
 *
 * Displays configuration information about the DPM system.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_config(int argc, char** argv);

/**
 * @brief Handler for unknown commands
 *
 * Displays an error message for unrecognized commands.
 *
 * @param command The unrecognized command string
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 1 to indicate failure
 */
int cmd_unknown(const char* command, int argc, char** argv);

/**
 * @brief Parses a command string into a Command enum value
 *
 * Converts a command string to the appropriate Command enum value
 * for internal routing.
 *
 * @param cmd_str The command string to parse
 * @return The corresponding Command enum value
 */
Command parse_command(const char* cmd_str);