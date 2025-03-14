/**
 * @file info.cpp
 * @brief Example DPM info module implementation
 *
 * Implements a simple DPM module that provides information about the DPM system.
 * This module demonstrates how to implement the required module interface and
 * interact with the DPM core through configuration functions.
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

// Implementation of the info module
// This module provides information about the DPM system

#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <sys/utsname.h>

#include "include/infoFuncs.hpp"

/**
 * @def MODULE_VERSION
 * @brief Version information for the info module
 *
 * Defines the version string that will be returned by dpm_module_get_version()
 */
#define MODULE_VERSION "0.1.0"

/**
 * @brief Returns the module version string
 *
 * Required implementation of the DPM module interface that provides
 * version information for the info module.
 *
 * @return Const char pointer to the module version string
 */
extern "C" const char* dpm_module_get_version(void) {
    return MODULE_VERSION;
}

/**
 * @brief Returns the module description string
 *
 * Required implementation of the DPM module interface that provides
 * a human-readable description of the info module and its functionality.
 *
 * @return Const char pointer to the module description string
 */
extern "C" const char* dpm_get_description(void) {
    return "Provides information about the DPM system.";
}



/**
 * @brief Main entry point for the info module
 *
 * Required implementation of the DPM module interface that serves as the
 * primary execution point for the module. Parses the command and routes
 * execution to the appropriate handler function.
 *
 * @param command The command string to execute
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return 0 on success, non-zero on failure
 */
extern "C" int dpm_module_execute(const char* command, int argc, char** argv) {
    dpm_log(LOG_DEBUG, "Info module execution started.");

    Command cmd = parse_command(command);

    switch (cmd) {
        case CMD_VERSION:
            return cmd_version(argc, argv);

        case CMD_SYSTEM:
            return cmd_system(argc, argv);

        case CMD_CONFIG:
            return cmd_config(argc, argv);

        case CMD_HELP:
            return cmd_help(argc, argv);

        case CMD_UNKNOWN:
        default:
            return cmd_unknown(command, argc, argv);
    }
}

// If we're building in standalone mode, include the main function
#ifdef BUILD_STANDALONE
DPM_MODULE_STANDALONE_MAIN()
#endif // BUILD_STANDALONE