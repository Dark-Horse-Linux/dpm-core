/**
 * @file dpm_interface.cpp
 * @brief Implementation of DPM command-line interface functions
 *
 * Implements the functions that provide human-readable interaction with the
 * DPM core functionality, including module path validation and module listing
 * capabilities.
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

#include "dpm_interface.hpp"

/*
 *  DPM Interface methods.
 *
 *  These are for entry points for the DPM cli.
 *
 *  These are wrappers of DPM functionality that are meant to handle user
 *  view, turning error codes into human-presentable information, etc.
 *
 *  Also includes helpers related to the CLI.
 */

/**
 * Verify that the module path exists and is accessible.
 *
 * This function checks if the configured module path exists, is a directory,
 * and has the necessary read permissions.
 *
 * @param loader Reference to a ModuleLoader object that provides the module path
 *
 * @return 0 if the path exists and is accessible, 1 otherwise
 *
 * The function performs the following checks:
 * 1. Retrieves the module path from the loader
 * 2. Verifies that the path exists in the filesystem
 * 3. Confirms that the path is a directory
 * 4. Checks that the directory has read permissions
 *
 * If any check fails, an appropriate error message is displayed to stderr.
 */
int main_check_module_path(const ModuleLoader& loader)
{
    std::string path;
    loader.get_module_path(path);

    if (!std::filesystem::exists(path)) {
        std::cerr << "FATAL: modules.modules_path does not exist: " << path << std::endl;
        return 1;
    }

    if (!std::filesystem::is_directory(path)) {
        std::cerr << "FATAL: modules.modules_path is not a directory: " << path << std::endl;
        return 1;
    }

    try {
        auto perms = std::filesystem::status(path).permissions();
        if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
            std::cerr << "FATAL: Permission denied: " << path << std::endl;
            return 1;
        }
    } catch (const std::filesystem::filesystem_error&) {
        std::cerr << "FATAL: Permission denied: " << path << std::endl;
        return 1;
    }

    return 0;
}

/**
 * List all available and valid DPM modules.
 *
 * This function retrieves and displays a formatted table of available DPM modules
 * from the specified module path, including their versions and descriptions.
 *
 * @param loader Reference to a ModuleLoader object that provides access to modules
 *
 * @return 0 on success, 1 on failure
 *
 * The function performs the following operations:
 * 1. Gets the configured module path from the loader
 * 2. Retrieves a list of all potential modules in that path
 * 3. Validates each module by checking for required symbols
 * 4. Collects version and description information from valid modules
 * 5. Formats and displays the information in a tabular format
 *
 * If no modules are found or if no valid modules are found, appropriate
 * messages are displayed.
 *
 * Modules are considered valid if they expose all required interface
 * symbols as defined in module_interface.hpp.
 */
int main_list_modules(const ModuleLoader& loader)
{
    // initialize an empty modules list
    std::vector<std::string> modules;

    // initialize an empty path
    std::string path;

    // set the module path
    DPMErrorCategory get_path_error = loader.get_module_path(path);
    if ( get_path_error != DPMErrorCategory::SUCCESS ) {
        std::cerr << "Failed to get modules.modules_path" << std::endl;
        return 1;
    }

    DPMErrorCategory list_error = loader.list_available_modules(modules);
    if (list_error != DPMErrorCategory::SUCCESS) {
        std::cerr << "FATAL: No modules found in modules.modules_path: " << path << std::endl;
        return 1;
    }

    if (modules.empty()) {
        std::cerr << "FATAL: No modules found in modules.modules_path: '" << path << "'." << std::endl;
        return 0;
    }

    // First pass: Identify valid modules
    std::vector<std::string> valid_modules;
    for (int i = 0; i < modules.size(); i++) {
        void* handle;
        DPMErrorCategory load_error = loader.load_module(modules[i], handle);
        if (load_error != DPMErrorCategory::SUCCESS) {
            continue;
        }

        std::vector<std::string> missing_symbols;
        DPMErrorCategory validate_error = loader.validate_module_interface(handle, missing_symbols);
        if (validate_error == DPMErrorCategory::SUCCESS) {
            valid_modules.push_back(modules[i]);
        }
        dlclose(handle);
    }

    if (valid_modules.empty()) {
        std::cerr << "FATAL: No valid modules found in modules.modules_path: '" << path << "'." << std::endl;
        return 0;
    }

    // Second pass: Collect module information and calculate column widths
    size_t max_name_length = 0;
    size_t max_version_length = 0;
    std::vector<std::string> versions(valid_modules.size(), "unknown");
    std::vector<std::string> descriptions(valid_modules.size(), "unknown");

    for (int i = 0; i < valid_modules.size(); i++) {
        void* module_handle;
        max_name_length = std::max(max_name_length, valid_modules[i].length());

        DPMErrorCategory load_error = loader.load_module(valid_modules[i], module_handle);
        if (load_error == DPMErrorCategory::SUCCESS) {
            // Get version
            std::string version = "unknown";
            DPMErrorCategory version_error = loader.get_module_version(module_handle, version);
            if (version_error == DPMErrorCategory::SUCCESS) {
                versions[i] = version;
                max_version_length = std::max(max_version_length, version.length());
            }

            // Get description
            std::string description = "unknown";
            DPMErrorCategory desc_error = loader.get_module_description(module_handle, description);
            if (desc_error == DPMErrorCategory::SUCCESS) {
                descriptions[i] = description;
            }

            dlclose(module_handle);
        }
    }

    const int column_spacing = 4;

    // Display the table header
    std::cout << "\nAvailable modules in modules.modules_path: '" << path << "':" << std::endl << std::endl;
    std::cout << std::left << std::setw(max_name_length + column_spacing) << "MODULE"
              << std::setw(max_version_length + column_spacing) << "VERSION"
              << "DESCRIPTION" << std::endl;

    // Display the table rows
    for (int i = 0; i < valid_modules.size(); i++) {
        std::cout << std::left << std::setw(max_name_length + column_spacing) << valid_modules[i]
                  << std::setw(max_version_length + column_spacing) << versions[i]
                  << descriptions[i] << std::endl;
    }

    return 0;
}
