#pragma once

#include "cli_parsers.hpp"
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <filesystem>

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
 * @brief Handler for the help command
 *
 * Displays information about available commands in the build module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);