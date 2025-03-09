/**
 * @file buildFuncs.hpp
 * @brief Header file for the build module support functions
 *
 * Defines functions and enumerations for the build module which creates
 * DPM packages according to specification.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <getopt.h>
#include <dpmdk/include/CommonModuleAPI.hpp>

/**
 * @enum Command
 * @brief Enumeration of supported commands for the build module
 */
enum Command {
    CMD_UNKNOWN,    /**< Unknown or unsupported command */
    CMD_HELP,       /**< Display help information */
    CMD_CREATE      /**< Create a new DPM package */
};

/**
 * @struct BuildOptions
 * @brief Structure to store parsed command-line options for the build module
 */
struct BuildOptions {
    std::string output_dir;        /**< Directory where to save the built package */
    std::string contents_dir;      /**< Directory with package contents */
    std::string metadata_dir;      /**< Directory with package metadata */
    std::string hooks_dir;         /**< Directory with package hooks */
    std::string package_name;      /**< Name of the package to build */
    std::string signature_key;     /**< Path to the GPG key for signing the package */
    bool force;                    /**< Flag to force package creation even if warnings occur */
    bool verbose;                  /**< Flag for verbose output */

    // Constructor with default values
    BuildOptions() :
        output_dir("."),
        contents_dir(""),
        metadata_dir(""),
        hooks_dir(""),
        package_name(""),
        signature_key(""),
        force(false),
        verbose(false) {}
};

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the build module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);

/**
 * @brief Handler for the create command
 *
 * Processes arguments and creates a DPM package.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_create(int argc, char** argv);

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

/**
 * @brief Parses command-line arguments for the create command
 *
 * Processes command-line arguments and populates a BuildOptions structure.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @param options Reference to BuildOptions structure to populate
 * @return 0 on success, non-zero on failure
 */
int parse_create_options(int argc, char** argv, BuildOptions& options);

/**
 * @brief Validates the build options
 *
 * Ensures that all required options are provided and valid.
 *
 * @param options The BuildOptions structure to validate
 * @return 0 if options are valid, non-zero otherwise
 */
int validate_build_options(const BuildOptions& options);