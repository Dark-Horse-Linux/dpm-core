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

int main_check_module_path(const ModuleLoader& loader)
{
    std::string path;
    loader.get_module_path(path);

    if (!std::filesystem::exists(path)) {
        dpm_con(FATAL, ("modules.modules_path does not exist: " + path).c_str());
        return 1;
    }

    if (!std::filesystem::is_directory(path)) {
        dpm_con(FATAL, ("modules.modules_path is not a directory: " + path).c_str());
        return 1;
    }

    try {
        auto perms = std::filesystem::status(path).permissions();
        if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
            dpm_con(FATAL, ("Permission denied: " + path).c_str());
            return 1;
        }
    } catch (const std::filesystem::filesystem_error&) {
        dpm_con(FATAL, ("Permission denied: " + path).c_str());
        return 1;
    }

    return 0;
}

int main_list_modules(const ModuleLoader& loader) {
    // initialize an empty modules list
    std::vector<std::string> modules;

    // initialize an empty path
    std::string path;

    // set the module path
    DPMErrorCategory get_path_error = loader.get_module_path(path);
    if (get_path_error != DPMErrorCategory::SUCCESS) {
        dpm_con(LoggingLevels::FATAL, "Failed to get modules.modules_path");
        return 1;
    }

    DPMErrorCategory list_error = loader.list_available_modules(modules);
    if (list_error != DPMErrorCategory::SUCCESS) {
        dpm_con(LoggingLevels::FATAL, ("No modules found in modules.modules_path: " + path).c_str());
        return 1;
    }

    if (modules.empty()) {
        dpm_con(LoggingLevels::FATAL, ("No modules found in modules.modules_path: '" + path + "'.").c_str());
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
        dpm_con(LoggingLevels::FATAL, "No valid DPM commands available.");
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

    // Print header with proper spacing
    std::cout << "Available DPM commands:\n" << std::endl;

    // Display the table header
    std::cout << std::left << std::setw(max_name_length + column_spacing) << "COMMAND"
              << std::setw(max_version_length + column_spacing) << "VERSION"
              << "DESCRIPTION" << std::endl;

    // Display the table rows
    for (int i = 0; i < valid_modules.size(); i++) {
        std::cout << std::left << std::setw(max_name_length + column_spacing) << valid_modules[i]
                  << std::setw(max_version_length + column_spacing) << versions[i]
                  << descriptions[i] << std::endl;
    }

    // Add a blank line before the usage note
    std::cout << std::endl;
    std::cout << "Use 'dpm <command> help' for detailed information about a specific command." << std::endl;
    std::cout << std::endl;

    return 0;
}

/**
 * @brief Displays usage information for DPM
 *
 * Shows a help message describing the available command-line options
 * and general usage information for the DPM utility.
 *
 * @return 0 on success
 */
int main_show_help() {
    std::cout << "Usage: dpm [options] [module-name] [module args...] [module-command] [command-args]\n\n"
              << "Options:\n\n"
              << "  -m, --module-path PATH   Path to DPM modules (overrides modules.modules_path in config)\n"
              << "  -c, --config-dir PATH    Path to DPM configuration directory\n"
              << "  -l, --list-modules       List available modules\n"
              << "  -h, --help               Show this help message\n\n"
              << "For module-specific help, use: dpm <module-name> help\n\n";
    return 0;
}

int main_execute_module( const ModuleLoader& loader, std::string module_name, std::string command ) {
    DPMErrorCategory execute_error = loader.execute_module(module_name, command);
    if (execute_error != DPMErrorCategory::SUCCESS) {
        // get the absolute module path
        std::string absolute_module_path = "";
        loader.get_module_path(absolute_module_path);

        // construct an error object
        FlexDPMError result = make_error( execute_error );
        result.module_name = module_name.c_str();
        result.module_path = absolute_module_path.c_str();

        // pair result with a message and return with the appropriate error code
        return handle_error(result);
    } else {
        return 0;
    }
}