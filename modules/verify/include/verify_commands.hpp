/**
 * @file verify_commands.hpp
 * @brief Header file for the verify module command handlers
 *
 * Defines functions and enumerations for the verify module which verifies
 * the integrity and signatures of installed packages.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <cstring>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <dlfcn.h>
#include <sys/stat.h>
#include <filesystem>

/**
 * @enum Command
 * @brief Enumeration of supported commands for the verify module
 */
enum Command {
    CMD_UNKNOWN,     /**< Unknown or unsupported command */
    CMD_HELP,        /**< Display help information */
    CMD_CHECKSUM,    /**< Verify package checksums */
    CMD_SIGNATURE    /**< Verify package signatures */
};

/**
 * @brief Handler for the checksum command
 *
 * Verifies the checksums of installed packages.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_checksum(int argc, char** argv);

/**
 * @brief Handler for the signature command
 *
 * Verifies the signatures of installed packages.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_signature(int argc, char** argv);

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the verify module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);

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