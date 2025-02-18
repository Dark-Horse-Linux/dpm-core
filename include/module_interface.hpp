#pragma once
#include <string>

/*
 *  Provides reserved symbol names we look for in modules.
 */

// Common interface for all DPM modules
extern "C" {
    // Module must export this symbol to be considered valid
    int dpm_module_execute(const char* command, int argc, char** argv);
}