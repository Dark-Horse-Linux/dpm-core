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
#include <sys/stat.h>
#include <dlfcn.h>

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
    const char* env_path = dpm_get_config("modules", "modules_path");
    return env_path ? env_path : "/usr/lib/dpm/modules/";
}

/**
 * @brief Standalone implementation of dpm_module_exists
 */
inline bool dpm_module_exists(const char* module_name) {
    if (!module_name) return false;

    // Get the module path
    const char* module_path = dpm_get_module_path();
    if (!module_path) return false;

    // Build path to the module
    std::string module_file = std::string(module_path) + "/" + module_name + ".so";

    // Check if file exists
    struct stat buffer;
    return (stat(module_file.c_str(), &buffer) == 0);
}

/**
 * @brief Standalone implementation of dpm_load_module
 */
inline int dpm_load_module(const char* module_name, void** module_handle) {
    if (!module_name || !module_handle) return 1;

    // Get the module path
    const char* module_path = dpm_get_module_path();
    if (!module_path) return 1;

    // Build path to the module
    std::string module_file = std::string(module_path) + "/" + module_name + ".so";

    // Check if the file exists
    if (!dpm_module_exists(module_name)) return 1;

    // Load the module
    *module_handle = dlopen(module_file.c_str(), RTLD_LAZY);
    if (!*module_handle) return 1;

    return 0;
}

/**
 * @brief Standalone implementation of dpm_symbol_exists
 */
inline bool dpm_symbol_exists(void* module_handle, const char* symbol_name) {
    if (!module_handle || !symbol_name) return false;

    // Clear any error
    dlerror();

    // Look up the symbol
    void* symbol = dlsym(module_handle, symbol_name);

    // Check for errors
    const char* error = dlerror();
    if (error) return false;

    return (symbol != NULL);
}

/**
 * @brief Standalone implementation of dpm_execute_symbol
 */
inline int dpm_execute_symbol(void* module_handle, const char* symbol_name, void* args) {
    if (!module_handle || !symbol_name) return 1;

    // Clear any error
    dlerror();

    // Look up the symbol
    void* symbol = dlsym(module_handle, symbol_name);

    // Check for errors
    const char* error = dlerror();
    if (error || !symbol) return 1;

    // Cast to function pointer and call
    typedef int (*FunctionPtr)(void*);
    FunctionPtr func = (FunctionPtr)symbol;

    return func(args);
}

/**
 * @brief Standalone implementation of dpm_unload_module
 */
inline void dpm_unload_module(void* module_handle) {
    if (module_handle) {
        dlclose(module_handle);
    }
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