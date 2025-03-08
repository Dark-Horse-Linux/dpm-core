/**
 * @file ModuleLoader.hpp
 * @brief Dynamic module loading and management for DPM
 *
 * Defines the ModuleLoader class which is responsible for finding, loading,
 * validating, and executing DPM modules. It handles the dynamic loading of
 * shared objects and ensures they conform to the expected module interface.
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

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cstring>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>

#include "error.hpp"
#include "module_interface.hpp"

/**
 * @class ModuleLoader
 * @brief Handles dynamic loading and management of DPM modules
 *
 * Provides functionality for discovering, loading, validating, and executing
 * DPM modules from shared object files. Ensures that modules conform to the
 * required interface before allowing their execution.
 */
class ModuleLoader {
    public:
        /**
         * @brief Constructor
         *
         * Initializes a new ModuleLoader with the specified module path.
         *
         * @param module_path Directory path where DPM modules are located (defaults to /usr/lib/dpm/modules/)
         */
        explicit ModuleLoader(std::string module_path = "/usr/lib/dpm/modules/");

        /**
         * @brief Lists available modules in the module path
         *
         * Populates a vector with the names of available modules found in
         * the configured module path.
         *
         * @param modules Vector to populate with module names
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory list_available_modules(std::vector<std::string>& modules) const;

        /**
         * @brief Gets the configured module path
         *
         * Retrieves the directory path where modules are located.
         *
         * @param path Reference to store the module path
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory get_module_path(std::string& path) const;

        /**
         * @brief Loads a module by name
         *
         * Attempts to dynamically load a module from the configured module path.
         *
         * @param module_name Name of the module to load
         * @param module_handle Reference to store the loaded module handle
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory load_module(const std::string& module_name, void*& module_handle) const;

        /**
         * @brief Executes a module with the specified command
         *
         * Loads a module and executes its main entry point with the given command.
         *
         * @param module_name Name of the module to execute
         * @param command Command string to pass to the module
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory execute_module(const std::string& module_name, const std::string& command) const;

        /**
         * @brief Gets a module's version information
         *
         * Retrieves the version string from a loaded module.
         *
         * @param module_handle Handle to a loaded module
         * @param version Reference to store the version string
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory get_module_version(void* module_handle, std::string& version) const;

        /**
         * @brief Gets a module's description
         *
         * Retrieves the description string from a loaded module.
         *
         * @param module_handle Handle to a loaded module
         * @param description Reference to store the description string
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory get_module_description(void* module_handle, std::string& description) const;

        /**
         * @brief Validates a module's interface
         *
         * Checks if a loaded module exports all required symbols as defined
         * in the module_interface.
         *
         * @param module_handle Handle to a loaded module
         * @param missing_symbols Reference to store names of any missing required symbols
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory validate_module_interface(void* module_handle, std::vector<std::string>& missing_symbols) const;

        /**
         * @brief Checks module path validity
         *
         * Verifies that the configured module path exists, is a directory,
         * and has the necessary permissions.
         *
         * @return DPMErrorCategory indicating success or failure
         */
        DPMErrorCategory check_module_path() const;

    private:
        /**
         * @brief Directory path where modules are located
         */
        std::string _module_path;
};