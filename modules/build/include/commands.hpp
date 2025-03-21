#pragma once

#include "cli_parsers.hpp"
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <filesystem>
#include "package_staging.hpp"
#include <map>
#include <sstream>

/**
 * @brief Handler for the stage command
 *
 * Processes arguments and stages a DPM package.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_stage(int argc, char** argv);

/**
 * @brief Handler for the manifest command
 *
 * Generates or refreshes package manifest.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_manifest(int argc, char** argv);

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
 * @brief Handler for the help command
 *
 * Displays information about available commands in the build module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_stage_help(int argc, char** argv);

/**
 * @brief Handler for the manifest help command
 *
 * Displays information about manifest command options.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_manifest_help(int argc, char** argv);

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