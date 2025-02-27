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
#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

#include "error.hpp"
#include "module_interface.hpp"


class ModuleLoader {
    public:
        // initializer
        explicit ModuleLoader(std::string module_path = "/usr/lib/dpm/modules/");
        DPMErrorCategory list_available_modules(std::vector<std::string>& modules) const;
        DPMErrorCategory get_module_path(std::string& path) const;

        // Load and execute methods
        DPMErrorCategory load_module(const std::string& module_name, void*& module_handle) const;
        DPMErrorCategory execute_module(const std::string& module_name, const std::string& command) const;

        // Get module version
        DPMErrorCategory get_module_version(void* module_handle, std::string& version) const;

        // Get module description
        DPMErrorCategory get_module_description(void* module_handle, std::string& description) const;

        // Check if all required symbols from module_interface.hpp are exported by the module
        DPMErrorCategory validate_module_interface(void* module_handle, std::vector<std::string>& missing_symbols) const;

        // Helper method to check module path validity
        DPMErrorCategory check_module_path() const;

    private:
        std::string _module_path;
};