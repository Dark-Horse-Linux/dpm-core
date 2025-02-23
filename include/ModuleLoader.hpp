#pragma once
#include <string>
#include <vector>
#include "error.hpp"
#include "dpm_interface.hpp"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>
#include <module_interface.hpp>

// Forward declaration to avoid circular dependency
struct CommandArgs;

class ModuleLoader {
public:
    explicit ModuleLoader(std::string module_path = "/usr/lib/dpm/modules/");
    DPMError list_available_modules(std::vector<std::string>& modules) const;
    DPMError get_module_path(std::string& path) const;
    DPMError get_absolute_module_path(std::string& abs_path) const;

    // Load and execute methods
    DPMError load_module(const std::string& module_name, void*& module_handle) const;
    DPMError execute_module(void* module_handle, const std::string& command) const;

    // Get module version
    DPMError get_module_version(void* module_handle, std::string& version) const;

    // Get module description
    DPMError get_module_description(void* module_handle, std::string& description) const;

    // Check if all required symbols from module_interface.hpp are exported by the module
    DPMError validate_module_interface(void* module_handle, std::vector<std::string>& missing_symbols) const;

private:
    std::string module_path_;
};