/**
 * @file CommonModuleAPI.hpp
 * @brief Common module interface for DPM modules
 *
 * Defines the required interface that all DPM modules must implement,
 * including the function declarations and constants that every module needs.
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
#include <string>
#include <sys/stat.h>
#include <dlfcn.h>
#include "ModuleOperations.hpp"

/**
 * @brief Fatal log level constant
 *
 * Used for critical errors that require immediate attention and
 * typically result in program termination.
 */
const int LOG_FATAL = 0;

/**
 * @brief Error log level constant
 *
 * Used for error conditions that may allow the program to continue
 * execution but indicate a failure in some operation.
 */
const int LOG_ERROR = 1;

/**
 * @brief Warning log level constant
 *
 * Used for potentially harmful situations that don't affect normal
 * program execution but may indicate problems.
 */
const int LOG_WARN  = 2;

/**
 * @brief Informational log level constant
 *
 * Used for general informational messages about program execution.
 */
const int LOG_INFO  = 3;

/**
 * @brief Debug log level constant
 *
 * Used for detailed debugging information during development or
 * troubleshooting.
 */
const int LOG_DEBUG = 4;

// Required functions that every DPM module must implement
extern "C" {
    /**
     * @brief Required module execution entry point
     *
     * Main entry point that is called by the DPM core when the module
     * is executed. Modules must implement this function to handle
     * commands and perform their specific functionality.
     *
     * @param command The command string to execute
     * @param argc Number of arguments
     * @param argv Array of argument strings
     * @return 0 on success, non-zero on failure
     */
    int dpm_module_execute(const char* command, int argc, char** argv);

    /**
     * @brief Module version information function
     *
     * Returns the version information for the module. This information
     * is displayed when listing available modules.
     *
     * @return A string containing the module's version
     */
    const char* dpm_module_get_version(void);

    /**
     * @brief Module description function
     *
     * Returns a human-readable description of the module's functionality.
     * This information is displayed when listing available modules.
     *
     * @return A string containing the module's description
     */
    const char* dpm_get_description(void);
}

// DPM core functions available for modules to call
extern "C" {
    /**
     * @brief Configuration access function
     *
     * Allows modules to access DPM's configuration values by section and key.
     *
     * @param section The configuration section name
     * @param key The configuration key within the section
     * @return The configuration value as a string, or NULL if not found
     */
    const char* dpm_get_config(const char* section, const char* key);

    /**
     * @brief Logging function
     *
     * Allows modules to log messages through DPM's logging system.
     *
     * @param level The log level (LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG)
     * @param message The message to log
     */
    void dpm_log(int level, const char* message);

    /**
     * @brief Console logging function
     *
     * Allows modules to log messages to the console only, bypassing the file logging.
     * This is useful for user-facing output that doesn't need to be recorded in logs.
     *
     * @param level The log level (LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG)
     * @param message The message to log
     */
    void dpm_con(int level, const char* message);

    /**
     * @brief Sets the logging level
     *
     * Allows modules to set the logging level used by the DPM logging system.
     * This is useful for implementing verbose modes in modules.
     *
     * @param level The log level (LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG)
     */
    void dpm_set_logging_level(int level);

    /**
     * @brief Gets the module path
     *
     * Returns the path where DPM modules are located, as determined by
     * command-line arguments, configuration files, or defaults.
     *
     * @return The module path
     */
    const char* dpm_get_module_path(void);
}

/**
 * @brief DPM core version definition
 *
 * Defines the version string for the DPM core system.
 */
#define DPM_VERSION "0.1.0"

// If we're building in standalone mode, include the standalone implementations
#ifdef BUILD_STANDALONE
#include "StandaloneModuleImpl.hpp"
#endif // BUILD_STANDALONE