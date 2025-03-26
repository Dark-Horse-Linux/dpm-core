/**
 * @file dpm_interface.hpp
 * @brief Interface declarations for the DPM command-line functionality
 *
 * Defines the public interface methods that provide human-readable interaction
 * with the DPM core functionality, including module path validation and
 * module listing capabilities.
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
#include <iostream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <dlfcn.h>
#include <getopt.h>

#include "error.hpp"
#include "ModuleLoader.hpp"
#include "dpm_interface_helpers.hpp"
#include "Logger.hpp"

#include <handlers.hpp>

/**
 * @defgroup dpm_interface DPM Interface Methods
 * @brief Interface methods for the DPM command-line tool
 *
 * These functions provide the human-readable interface for the DPM utility,
 * transforming error codes into user-friendly messages and implementing
 * commands that report system information.
 * @{
 */

/**
 * @brief Verifies that the module path exists and is accessible
 *
 * Checks if the configured module path exists, is a directory, and has
 * the necessary read permissions. If any check fails, an appropriate
 * error message is displayed.
 *
 * @param loader Reference to a ModuleLoader object that provides the module path
 * @return 0 if the path exists and is accessible, 1 otherwise
 */
int main_check_module_path(const ModuleLoader& loader);

/**
 * @brief Lists all available and valid DPM modules
 *
 * Retrieves and displays a formatted table of available DPM modules
 * from the specified module path, including their versions and descriptions.
 * Validates each module by checking for required symbols before including
 * it in the list.
 *
 * @param loader Reference to a ModuleLoader object that provides access to modules
 * @return 0 on success, 1 on failure
 */
int main_list_modules(const ModuleLoader& loader);

/**
 * @brief Displays usage information for DPM
 *
 * Shows a help message describing the available command-line options
 * and general usage information for the DPM utility.
 *
 * @return 0 on success
 */
int main_show_help();

/**
 * @brief Executes a DPM module with the specified command
 *
 * Attempts to execute the specified module with the given command string and
 * handles any errors that occur during execution. If the execution is successful,
 * returns 0. If an error occurs, constructs an appropriate error context and
 * processes it through the error handling system.
 *
 * @param loader Reference to a ModuleLoader object that provides access to modules
 * @param module_name Name of the module to execute
 * @param command Command string to pass to the module
 * @return 0 on successful execution, appropriate error code otherwise
 */
int main_execute_module( const ModuleLoader& loader, std::string module_name, std::string command );


/** @} */ // end of dpm_interface group