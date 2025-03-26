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
#include "Logger.hpp"
#include "LoggingLevels.hpp"
#include "module_interface.hpp"

/*
 *   DPM serves three functions:
 *       1. Find and load modules.
 *       2. Route commands to modules.
 *       3. Provide a module-agnostic unified interface for modules.
 */

// the default behaviour if dpm is executed without being told to do anything
int default_behavior(const ModuleLoader& loader)
{
    return main_show_help();
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
int main( int argc, char* argv[] )
{
    // process the arguments supplied to DPM and provide
    // an object that contains them for command and routing
    // processing
    CommandArgs args = parse_args( argc, argv );

    // Set the configuration directory path (CLI argument takes precedence over defaults)
    if ( !args.config_dir.empty() )
    {
        // args.config_dir was supplied so set it
        g_config_manager.setConfigDir( args.config_dir );
    } else {
        // args.config_dir was not supplied, so fall back to default path
        g_config_manager.setConfigDir( DPMDefaults::CONFIG_DIR );
    }

    // Load configuration files
    bool config_loaded = g_config_manager.loadConfigurations();
    if (!config_loaded)
    {
        // failed to load any configuration files, so alert the user
        dpm_con( ERROR, ("Warning: No configuration files present or loaded from '" + g_config_manager.getConfigDir() + "*.conf', reverting to defaults.").c_str());
    }

    // Configure logger (CLI args > config > defaults)
    // Check configuration for log settings
    bool config_write_to_log = g_config_manager.getConfigBool("logging", "write_to_log", DPMDefaults::write_to_log);
    std::string config_log_file = g_config_manager.getConfigString("logging", "log_file", DPMDefaults::LOG_FILE);

    // Parse log_level from config using the new method
    std::string log_level_str = g_config_manager.getConfigString("logging", "log_level", "INFO");
    LoggingLevels config_log_level = Logger::stringToLogLevel(log_level_str, DPMDefaults::LOG_LEVEL);

    // Configure global logger instance
    g_logger.setLogLevel(config_log_level);
    g_logger.setWriteToLog(config_write_to_log);
    g_logger.setLogFile(config_log_file);

    // If help is requested, show it and exit - handle this early before any logging is needed
    if (args.show_help) {
        return main_show_help();
    }

    // If list modules is requested, handle it early too
    if (args.list_modules) {
        // Determine the module path (CLI arg > config > default)
        std::string module_path;

        // If CLI argument was provided, use it
        if (!args.module_path.empty())
        {
            module_path = args.module_path;
        } else {
            // Otherwise, check configuration file
            const char* config_module_path = g_config_manager.getConfigValue("modules", "module_path");
            if (config_module_path)
            {
                module_path = config_module_path;
            } else {
                // use default if nothing else is available
                module_path = DPMDefaults::MODULE_PATH;
            }
        }

        // create a module loader object with the determined path
        ModuleLoader loader(module_path);

        // check the module path for the loader object
        int path_check_result = main_check_module_path(loader);
        if (path_check_result != 0)
        {
            // exit if there's an error and ensure
            // it has an appropriate return code
            return path_check_result;
        }

        return main_list_modules(loader);
    }

    // Determine the module path (CLI arg > config > default)
    std::string module_path;

    // If CLI argument was provided, use it
    if (!args.module_path.empty())
    {
        module_path = args.module_path;
    } else {
        // Otherwise, check configuration file
        const char* config_module_path = g_config_manager.getConfigValue("modules", "module_path");
        if (config_module_path)
        {
            module_path = config_module_path;
        } else {
            // use default if nothing else is available
            module_path = DPMDefaults::MODULE_PATH;
        }
    }

    // create a module loader object with the determined path
    ModuleLoader loader(module_path);

    // check the module path for the loader object
    int path_check_result = main_check_module_path(loader);
    if (path_check_result != 0)
    {
        // exit if there's an error and ensure
        // it has an appropriate return code
        return path_check_result;
    }

    // if no module is provided to execute, then trigger the default
    // behaviour (show help)
    if (args.module_name.empty()) {
        return default_behavior(loader);
    }

    // execute the module
    int return_code = main_execute_module(loader, args.module_name, args.command);

    return return_code;
}