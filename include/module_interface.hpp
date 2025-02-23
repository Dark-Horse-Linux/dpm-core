#pragma once
#include <string>
#include <vector>

/*
 *  Provides reserved symbol names we look for in modules.
 */

// Define required symbols in one place
namespace module_interface {
    // This is the single source of truth for required module symbols
    static const std::vector<std::string> required_symbols = {
        "dpm_module_execute",
        "dpm_module_get_version",
        "dpm_get_description"
    };
}

// Common interface for all DPM modules
extern "C" {
    // Module must export this symbol to be considered valid
    int dpm_module_execute(const char* command, int argc, char** argv);

    // Module version information
    const char* dpm_module_get_version(void);

    // Module description information
    const char* dpm_get_description(void);
}