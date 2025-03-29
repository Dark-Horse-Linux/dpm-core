/**
* @file CommonModuleAPI.cpp
 * @brief Implementation for DPMDK functions
 *
 * Implements the module utility functions provided by the DPMDK library
 * for module interoperability and loading.
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
#include "CommonModuleAPI.hpp"

// Only define these functions when not in standalone mode
#ifndef BUILD_STANDALONE

extern "C" bool dpm_module_exists(const char* module_name) {
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

extern "C" int dpm_load_module(const char* module_name, void** module_handle) {
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

extern "C" bool dpm_symbol_exists(void* module_handle, const char* symbol_name) {
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

extern "C" int dpm_execute_symbol(void* module_handle, const char* symbol_name, void* args) {
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

extern "C" void dpm_unload_module(void* module_handle) {
    if (module_handle) {
        dlclose(module_handle);
    }
}

#endif // BUILD_STANDALONE