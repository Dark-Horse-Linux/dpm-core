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

// prints error message and returns error code
int print_error(DPMError error, const std::string& module_name, const std::string& module_path) {
    switch (error) {
        case DPMError::SUCCESS:
            return 0;
        case DPMError::PATH_NOT_FOUND:
            std::cerr << "Module path not found: " << module_path << std::endl;
            return 1;
        case DPMError::PATH_NOT_DIRECTORY:
            std::cerr << "Module path is not a directory: " << module_path << std::endl;
            return 1;
        case DPMError::PERMISSION_DENIED:
            std::cerr << "Permission denied accessing module: " << module_name << std::endl;
            return 1;
        case DPMError::MODULE_NOT_FOUND:
            std::cerr << "Module not found: " << module_name << std::endl;
            return 1;
        case DPMError::MODULE_LOAD_FAILED:
            std::cerr << "Failed to load module: " << module_name << std::endl;
            return 1;
        case DPMError::INVALID_MODULE:
            std::cerr << "Invalid module format: " << module_name << std::endl;
            return 1;
        default:
            std::cerr << "Unknown error executing module: " << module_name << std::endl;
            return 1;
    }
}

// the default behaviour if dpm is executed without being told to do anything
int default_behavior(const ModuleLoader& loader)
{
    return main_list_modules(loader);
}

// entry point for the DPM utility
int main( int argc, char* argv[] )
{
    // process the arguments suppplied to DPM and provide
    // an object that contains them for command and routing
    // processing
    auto args = parse_args( argc, argv );

    // create a module loader object at the supplied or default path
    // TODO: the default is set in the header instead of the
    //   implementation, fix that
    ModuleLoader loader( args.module_path );

    // check the module path for the loader object
    int path_check_result = main_check_module_path( loader );
    if ( path_check_result != 0 ) {
        // exit if there's an error and ensure
        // it has an appropriate return code
        return 1;
    }

    // if no module is provided to execute, then trigger the default
    // dpm behaviour
    if ( args.module_name.empty() ) {
        return default_behavior( loader );
    }

    DPMError execute_error = loader.execute_module(args.module_name, args.command);
    return print_error(execute_error, args.module_name, args.module_path);
}