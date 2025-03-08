/**
 * @file dpm_interface_helpers.hpp
 * @brief Helper functions for DPM command-line interface
 *
 * Provides utility functions for command-line argument parsing and
 * data structures for representing command arguments in a structured format.
 * These helpers are used by the main DPM interface to process user input.
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

#include <string>
#include <iostream>
#include <getopt.h>

#include "Logger.hpp"
#include "LoggingLevels.hpp"
#include "DPMDefaults.hpp"

/**
 * @struct CommandArgs
 * @brief Structure for storing parsed command-line arguments
 *
 * Contains fields for all command-line arguments that can be passed to
 * the DPM utility, providing a structured way to access them throughout
 * the application.
 */
struct CommandArgs {
    std::string module_path;  /**< Path to the directory containing DPM modules */
    std::string config_dir;   /**< Path to the directory containing configuration files */
    std::string module_name;  /**< Name of the module to execute */
    std::string command;      /**< Command string to pass to the module */
    bool list_modules;        /**< Flag to indicate if modules should be listed */
    bool show_help;           /**< Flag to indicate if help message should be shown */
};

/**
 * @brief Parses command-line arguments into a CommandArgs structure
 *
 * Processes the arguments provided to DPM and organizes them into a
 * CommandArgs structure for easier access. Handles options like
 * --module-path, --config-dir, --list-modules, and --help, as well as module names
 * and module-specific arguments.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of C-style strings containing the arguments
 * @return A CommandArgs structure containing the parsed arguments
 */
CommandArgs parse_args(int argc, char* argv[]);