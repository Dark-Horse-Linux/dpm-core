/**
 * @file ModuleOperations.hpp
 * @brief C++ interface for module operations with direct passthrough
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#pragma once

#include <string>
#include <filesystem>
#include <dlfcn.h>
#include <sys/stat.h>
#include <dpmdk/include/CommonModuleAPI.hpp>

/**
 * @brief Checks if a module exists
 *
 * Verifies if a module exists at the configured module path.
 *
 * @param module_name Name of the module to check
 * @return true if the module exists, false otherwise
 */
bool dpm_module_exists(const char* module_name);

/**
 * @brief Loads a DPM module
 *
 * Attempts to load a module from the configured module path.
 *
 * @param module_name Name of the module to load
 * @param module_handle Pointer to store the loaded module handle
 * @return 0 on success, non-zero on failure
 */
int dpm_load_module(const char* module_name, void** module_handle);

/**
 * @brief Checks if a symbol exists in a module
 *
 * Verifies if a specific symbol exists in a loaded module.
 *
 * @param module_handle Handle to a loaded module
 * @param symbol_name Name of the symbol to check
 * @return true if the symbol exists, false otherwise
 */
bool dpm_symbol_exists(void* module_handle, const char* symbol_name);

/**
 * @brief Executes a symbol in a module with direct argument passthrough
 *
 * Template function that directly passes any number of arguments to the target function.
 *
 * @tparam Args Variable argument types to pass to the function
 * @param module_handle Handle to a loaded module
 * @param symbol_name Name of the symbol to execute
 * @param args Arguments to pass to the function
 * @return Return value from the executed function
 */
template<typename... Args>
int dpm_execute_symbol(void* module_handle, const char* symbol_name, Args&&... args) {
    if (!module_handle || !symbol_name) return 1;

    // Clear any previous error
    dlerror();

    // Look up the symbol with the correct function signature
    typedef int (*FunctionPtr)(Args...);
    FunctionPtr func = reinterpret_cast<FunctionPtr>(dlsym(module_handle, symbol_name));

    // Check for errors
    const char* error = dlerror();
    if (error || !func) return 1;

    // Call the function with the provided arguments - direct passthrough
    return func(std::forward<Args>(args)...);
}

/**
 * @brief Unloads a module
 *
 * Frees resources used by a loaded module.
 *
 * @param module_handle Handle to a loaded module
 */
void dpm_unload_module(void* module_handle);