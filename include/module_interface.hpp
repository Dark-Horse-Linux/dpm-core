/**
 * @file module_interface.hpp
 * @brief Defines the interface for DPM modules
 *
 * Establishes the required symbols and common interface that all DPM modules
 * must implement to be loadable and executable by the core DPM system.
 * This forms the contract between the main DPM application and its modules.
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
#include <vector>
#include <iostream>

#include "ConfigManager.hpp"
#include "LoggingLevels.hpp"
#include "Logger.hpp"

/**
 * @namespace module_interface
 * @brief Namespace containing module interface definitions
 *
 * Provides reserved symbol names and interface definitions for DPM modules.
 */
namespace module_interface {
    /**
     * @brief List of required symbols that every valid DPM module must export
     *
     * These function names must be exported by a module for the module to be
     * considered valid and loadable by the DPM core system.
     */
    static const std::vector<std::string> required_symbols = {
        "dpm_module_execute",
        "dpm_module_get_version",
        "dpm_get_description"
    };
}

/**
 * @defgroup module_interface Common Interface for DPM Modules
 * @brief Functions that modules must implement and core functions available to modules
 * @{
 */
extern "C" {
    /**
     * @brief Main entry point for module execution
     *
     * This function must be implemented by all modules and serves as the
     * primary execution point when the module is invoked by DPM.
     *
     * @param command The command string to execute
     * @param argc Number of arguments provided
     * @param argv Array of argument strings
     * @return 0 on success, non-zero on failure
     */
    int dpm_module_execute(const char* command, int argc, char** argv);

    /**
     * @brief Provides module version information
     *
     * Returns a string containing the module's version information.
     * This is displayed when listing available modules.
     *
     * @return String containing the module's version
     */
    const char* dpm_module_get_version(void);

    /**
     * @brief Provides module description
     *
     * Returns a human-readable description of the module's functionality.
     * This is displayed when listing available modules.
     *
     * @return String containing the module's description
     */
    const char* dpm_get_description(void);

    /**
     * @brief Accesses configuration values
     *
     * Allows modules to retrieve configuration values from the DPM
     * configuration system. Implemented by the DPM core and available
     * to all modules.
     *
     * @param section The configuration section name
     * @param key The configuration key within the section
     * @return The configuration value as a string, or NULL if not found
     */
    const char* dpm_get_config(const char* section, const char* key);

    /**
     * @brief Logs messages through the DPM logging system
     *
     * Allows modules to log messages using the centralized DPM logging
     * system. Implemented by the DPM core and available to all modules.
     *
     * @param level The log level as an integer (0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG)
     * @param message The message to log
     */
    void dpm_log(int level, const char* message);

    /**
     * @brief Sets the logging level
     *
     * Allows modules to set the logging level used by the DPM logging system.
     * This is useful for implementing verbose modes in modules.
     *
     * @param level The log level as an integer (0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG)
     */
    void dpm_set_logging_level(int level);
}
/** @} */