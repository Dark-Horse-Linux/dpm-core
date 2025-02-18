#include <iostream>
#include <dlfcn.h>

#include "ModuleLoader.hpp"
#include "dpm_interface.hpp"

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

// entry point for the DPM utility
int main(int argc, char* argv[])
{
    auto args = parse_args(argc, argv);
    ModuleLoader loader(args.module_path);

    // check if the modules path even exists and return an error if not since we can't do anything
    if (auto result = main_check_module_path(loader); result != 0) {
        return result;
    }

    // if no modules are supplied, execute the default behaviour and exit
    if (args.module_name.empty()) {
        return default_behavior(loader);
    }

    // load the module specified
    void* module_handle = loader.load_module(args.module_name);
    if (!module_handle) {
        std::cerr << "Failed to load module: " << args.module_name << std::endl;
        return 1;
    }

    // Execute the module with the command string
    int result = loader.execute_module(module_handle, args.command);

    // Cleanup
    dlclose(module_handle);

    return result;
}