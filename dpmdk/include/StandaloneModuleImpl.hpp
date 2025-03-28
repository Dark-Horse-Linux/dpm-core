/**
 * @file StandaloneModuleImpl.hpp
 * @brief Standalone implementations for DPM modules
 *
 * Provides implementations of core DPM functions for standalone module builds,
 * allowing modules to be compiled and run independently of the main DPM system
 * for testing and development purposes.
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
#include <cstdlib>

// All implementations must be inline to prevent multiple definition errors when included in multiple files
/**
 * @brief Standalone implementation of dpm_log
 */
inline void dpm_log(int level, const char* message) {
    const char* level_str;
    switch (level) {
        case 0: level_str = "FATAL"; break;
        case 1: level_str = "ERROR"; break;
        case 2: level_str = "WARN"; break;
        case 3: level_str = "INFO"; break;
        case 4: level_str = "DEBUG"; break;
        default: level_str = "UNKNOWN"; break;
    }
    std::cout << "[" << level_str << "] " << message << std::endl;
}

/**
 * @brief Standalone implementation of dpm_con
 */
inline void dpm_con(int level, const char* message) {
    const char* level_str;
    switch (level) {
        case 0: level_str = "FATAL"; break;
        case 1: level_str = "ERROR"; break;
        case 2: level_str = "WARN"; break;
        case 3: level_str = "INFO"; break;
        case 4: level_str = "DEBUG"; break;
        default: level_str = "UNKNOWN"; break;
    }
    std::cout << "[" << level_str << "] " << message << std::endl;
}

/**
 * @brief Standalone implementation of dpm_get_config
 */
inline const char* dpm_get_config(const char* section, const char* key) {
    if (!section || !key) return nullptr;

    // Create environment variable name in format SECTION_KEY
    std::string env_name = std::string(section) + "_" + std::string(key);

    // Check if environment variable exists
    const char* env_value = getenv(env_name.c_str());
    return env_value; // Will be null if env var doesn't exist
}

/**
 * @brief Standalone implementation of dpm_set_logging_level
 */
inline void dpm_set_logging_level(int level) {
    std::cout << "[INFO] Verbosity level ignored, as all standalone executions have maximum verbosity" << std::endl;
}

/**
 * @brief Standalone implementation of dpm_get_module_path
 */
inline const char* dpm_get_module_path(void) {
    // Get from environment variable or use default
    const char* env_path = getenv("DPM_MODULE_PATH");
    return env_path ? env_path : "/usr/lib/dpm/modules/";
}

/**
 * @brief Standalone module main function
 *
 * Provides a main() function for standalone module builds that
 * initializes the environment and routes to the module's execute function.
 */
#define DPM_MODULE_STANDALONE_MAIN() \
int main(int argc, char** argv) { \
    /* Default to "help" if no command is provided */ \
    const char* command = "help"; \
    \
    /* If arguments are provided, use the first as command */ \
    if (argc > 1) { \
        command = argv[1]; \
        /* Shift arguments for the command handler but keep the original argc count */ \
        argv++; \
        argc--; \
    } \
    \
    return dpm_module_execute(command, argc, argv); \
}
/* End of file */