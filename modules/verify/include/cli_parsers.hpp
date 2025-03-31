#pragma once
#include <string>
#include <cstring>

/**
* @enum Command
 * @brief Enumeration of supported commands for the verify module
 */
enum Command {
    CMD_UNKNOWN,     /**< Unknown or unsupported command */
    CMD_HELP,        /**< Display help information */
    CMD_CHECKSUM,    /**< Verify package checksums */
    CMD_SIGNATURE,   /**< Verify package signatures */
    CMD_CHECK        /**< Check build module integration */
};

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

