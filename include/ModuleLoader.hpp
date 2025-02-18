#pragma once
#include <string>
#include <vector>
#include "error.hpp"

// Forward declaration to avoid circular dependency
struct CommandArgs;

class ModuleLoader {
public:
    explicit ModuleLoader(std::string module_path = "/usr/lib/dpm/modules/");
    DPMError check_module_path() const;
    std::pair<std::vector<std::string>, DPMError> list_available_modules() const;
    const std::string& get_module_path() const { return module_path_; }
    std::string get_absolute_module_path() const;

    // Split into two separate methods
    void* load_module(const std::string& module_name) const;
    int execute_module(void* module_handle, const std::string& command) const;

private:
    std::string module_path_;
};