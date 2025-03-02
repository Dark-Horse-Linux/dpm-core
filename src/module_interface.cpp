/**
 * @file module_interface.cpp
 * @brief Implementation of the module interface functions
 *
 * Provides the implementation of functions declared in the module interface
 * that are part of the DPM core, such as callback functions available to modules.
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

#include "module_interface.hpp"

/**
 * @brief Function that modules can call to access DPM core functionality
 *
 * This function is exported by the DPM executable and can be called by
 * dynamically loaded modules to access services or features of the core application.
 *
 * Note: In order for a module to use this functionality, it requires a forward declaration in the module:
 * // Example:
 * // Declaration of the DPM core function we want to call
 *  extern "C" int dpm_core_callback(const char* action, const char* data);
 *
 * @param action The specific action or operation being requested
 * @param data Additional data or parameters needed for the action
 * @return Result code indicating success (0) or failure (non-zero)
 */
extern "C" int dpm_core_callback(const char* action, const char* data) {
    // Implementation here
    if (!action) {
        std::cerr << "Error: Module callback received null action" << std::endl;
        return 1;
    }

    std::string action_str(action);

    if (action_str == "log") {
        if (data) {
            std::cout << "Module log: " << data << std::endl;
        }
        return 0;
    }
    else if (action_str == "get_config") {
        if (!data) {
            std::cerr << "Error: get_config requires section.key format" << std::endl;
            return 1;
        }

        // Parse the section.key format
        std::string data_str(data);
        size_t dot_pos = data_str.find('.');

        if (dot_pos == std::string::npos) {
            std::cerr << "Error: Invalid config format. Use section.key" << std::endl;
            return 1;
        }

        std::string section = data_str.substr(0, dot_pos);
        std::string key = data_str.substr(dot_pos + 1);

        // Get the configuration value
        const char* value = g_config_manager.getConfigValue(section.c_str(), key.c_str());

        // Log the request for debugging purposes
        std::cout << "Module requested config: " << data << " = "
                  << (value ? value : "not found") << std::endl;

        return 0;
    }

    std::cerr << "Error: Unknown module callback action: " << action_str << std::endl;
    return 1;
}

/**
 * @brief Function that provides direct access to configuration values
 *
 * This function allows modules to get configuration values directly.
 * It returns the string value from the configuration or nullptr if not found.
 *
 * @param section The configuration section name
 * @param key The configuration key within the section
 * @return The configuration value or nullptr if not found
 */
extern "C" const char* dpm_get_config(const char* section, const char* key) {
    return g_config_manager.getConfigValue(section, key);
}
