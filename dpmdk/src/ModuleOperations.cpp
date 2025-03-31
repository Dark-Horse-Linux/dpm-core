/**
* @file ModuleOperations.cpp
 * @brief Implementation of C++ interface for module operations
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "ModuleOperations.hpp"


bool dpm_module_exists(const char* module_name) {
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

int dpm_load_module(const char* module_name, void** module_handle) {
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

bool dpm_symbol_exists(void* module_handle, const char* symbol_name) {
    if (!module_handle || !symbol_name) return false;

    // Clear any error
    dlerror();

    // Look up the symbol
    void* symbol = dlsym(module_handle, symbol_name);

    // Check for errors
    const char* error = dlerror();
    if (error) return false;

    return (symbol != nullptr);
}

void dpm_unload_module(void* module_handle) {
    if (module_handle) {
        dlclose(module_handle);
    }
}