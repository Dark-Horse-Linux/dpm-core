/**
 * @file dpm.cpp
 * @brief Main entry point for the Dark Horse Package Manager (DPM)
 *
 * Implements the core command-line interface and module routing functionality
 * for the DPM utility, handling argument parsing, module loading, and execution.
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

#include <iostream>
#include <dlfcn.h>

#include <handlers.hpp>
#include "ModuleLoader.hpp"
#include "dpm_interface.hpp"
#include "dpm_interface_helpers.hpp"
#include "error.hpp"
#include "ConfigManager.hpp"

/*
 *   DPM serves three functions:
 *       1. Find and load modules.
 *       2. Route commands to modules.
 *       3. Provide a module-agnostic unified interface for modules.
 */

// the default behaviour if dpm is executed without being told to do anything
int default_behavior(const ModuleLoader& loader)
{
    return main_list_modules(loader);
}

/**
 * @brief Entry point for the DPM utility
 *
 * Processes command-line arguments, loads and executes the appropriate module,
 * and handles any errors that occur during execution.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of C-style strings containing the arguments
 * @return Exit code indicating success (0) or failure (non-zero)
 */
int main(int argc, char* argv[])
{
    // Load configuration files
    if (!g_config_manager.loadConfigurations()) {
        std::cerr << "Warning: No configuration files present or loaded from '" <<  DPMDefaultPaths::CONFIG_DIR << "*.conf', reverting to defaults." << std::endl;
        // Continue execution, as we might be able to use default values
    }

    // process the arguments suppplied to DPM and provide
    // an object that contains them for command and routing
    // processing - this will include any module_path from CLI
    auto args = parse_args(argc, argv);

    // Determine the module path (CLI arg > config > default)
    std::string module_path;

    // If CLI argument was provided, use it
    if (!args.module_path.empty()) {
        module_path = args.module_path;
    } else {
        // Otherwise, check configuration file
        const char* config_module_path = g_config_manager.getConfigValue("modules", "module_path");
        if (config_module_path) {
            module_path = config_module_path;
        }
        // Finally, use default if nothing else is available
        else {
            module_path = DPMDefaultPaths::MODULE_PATH;
        }
    }

    // create a module loader object with the determined path
    ModuleLoader loader(module_path);

    // check the module path for the loader object
    int path_check_result = main_check_module_path(loader);
    if (path_check_result != 0) {
        // exit if there's an error and ensure
        // it has an appropriate return code
        return 1;
    }

    // if no module is provided to execute, then trigger the default
    // dpm behaviour
    if (args.module_name.empty()) {
        return default_behavior(loader);
    }

    // execute the module
    DPMErrorCategory execute_error = loader.execute_module(args.module_name, args.command);

    std::string extracted_path;
    loader.get_module_path(extracted_path);

    FlexDPMError result = make_error(execute_error);
    result.module_name = args.module_name.c_str();
    result.module_path = extracted_path.c_str();

    // pair result with a message and exit with the appropriate error code
    return handle_error(result);
}
