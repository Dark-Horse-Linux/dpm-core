/**
 * @file package_operations.cpp
 * @brief Implementation of package operation functions
 *
 * Implements functions for extracting and verifying components from DPM packages.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "package_operations.hpp"


int get_component_from_package(const std::string& package_path,
                              const std::string& component_name,
                              unsigned char** data,
                              size_t* data_size)
{
    // Validate input parameters
    if (package_path.empty() || component_name.empty() || !data || !data_size) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to get_component_from_package");
        return 1;
    }

    // Initialize output parameters
    *data = nullptr;
    *data_size = 0;

    // Check if the package file exists
    if (!std::filesystem::exists(package_path)) {
        dpm_log(LOG_ERROR, ("Package file not found: " + package_path).c_str());
        return 1;
    }

    // Load the build module
    void* build_module = nullptr;
    int result = check_and_load_build_module(build_module);
    if (result != 0 || build_module == nullptr) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return 1;
    }

    dpm_log(LOG_DEBUG, ("Extracting " + component_name + " from package: " + package_path).c_str());

    // Call the function from the build module
    bool success = dpm_execute_symbol(build_module, "get_file_from_package_file",
                                    package_path.c_str(), component_name.c_str(),
                                    data, data_size);

    // Unload the build module
    dpm_unload_module(build_module);

    // Check if the function call was successful
    if (!success || *data == nullptr || *data_size == 0) {
        dpm_log(LOG_ERROR, ("Failed to extract " + component_name + " from package").c_str());
        return 1;
    }

    dpm_log(LOG_DEBUG, ("Successfully extracted " + component_name + " (" +
            std::to_string(*data_size) + " bytes)").c_str());

    return 0;
}

/**
 * @brief Extracts a file from a component archive
 *
 * Extracts a specific file from a component archive that has already been loaded into memory.
 * Uses the build module's get_file_from_memory_loaded_archive function.
 *
 * @param component_data Pointer to the component archive data in memory
 * @param component_size Size of the component archive in memory
 * @param filename Name of the file to extract from the component
 * @param data Pointer to a pointer that will be populated with the file data
 * @param data_size Pointer to a size_t that will be populated with the size of the file data
 * @return 0 on success, non-zero on failure
 */
int get_file_from_component(const unsigned char* component_data,
                           size_t component_size,
                           const std::string& filename,
                           unsigned char** data,
                           size_t* data_size)
{
    // Validate input parameters
    if (!component_data || component_size == 0 || filename.empty() || !data || !data_size) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to get_file_from_component");
        return 1;
    }

    // Initialize output parameters
    *data = nullptr;
    *data_size = 0;

    // Load the build module
    void* build_module = nullptr;
    int result = check_and_load_build_module(build_module);
    if (result != 0 || build_module == nullptr) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return 1;
    }

    dpm_log(LOG_DEBUG, ("Extracting file '" + filename + "' from component archive").c_str());

    // Call the function from the build module
    bool success = dpm_execute_symbol(build_module, "get_file_from_memory_loaded_archive",
                                    component_data, component_size,
                                    filename.c_str(),
                                    data, data_size);

    // Unload the build module
    dpm_unload_module(build_module);

    // Check if the function call was successful
    if (!success || *data == nullptr || *data_size == 0) {
        dpm_log(LOG_ERROR, ("Failed to extract file '" + filename + "' from component archive").c_str());
        return 1;
    }

    dpm_log(LOG_DEBUG, ("Successfully extracted file '" + filename + "' (" +
            std::to_string(*data_size) + " bytes)").c_str());

    return 0;
}