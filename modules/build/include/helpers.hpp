/**
 * @file helpers.hpp
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
#include <cstdlib>
#include <wordexp.h>
#include <dpmdk/include/CommonModuleAPI.hpp>

/**
 * @brief Expands environment variables and tildes in a path
 *
 * Uses wordexp to handle shell-like expansions in paths,
 * including environment variables, tildes, and wildcards.
 *
 * @param path The path string to expand
 * @return The expanded path, or the original path if expansion failed
 */
std::string expand_path(const std::string& path);

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

