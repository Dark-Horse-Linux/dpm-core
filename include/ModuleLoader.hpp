#pragma once
#include <string>
#include <vector>
#include "error.hpp"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>
#include "module_interface.hpp"

class ModuleLoader {
public:
    explicit ModuleLoader(std::string module_path = "/usr/lib/dpm/modules/");
    DPMError list_available_modules(std::vector<std::string>& modules) const;
    DPMError get_module_path(std::string& path) const;

    // Load and execute methods
    DPMError load_module(const std::string& module_name, void*& module_handle) const;
    DPMError execute_module(const std::string& module_name, const std::string& command) const;

    // Get module version
    DPMError get_module_version(void* module_handle, std::string& version) const;

    // Get module description
    DPMError get_module_description(void* module_handle, std::string& description) const;

    // Check if all required symbols from module_interface.hpp are exported by the module
    DPMError validate_module_interface(void* module_handle, std::vector<std::string>& missing_symbols) const;

private:
    std::string _module_path;
};