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
int main( int argc, char* argv[] )
{
    // process the arguments suppplied to DPM and provide
    // an object that contains them for command and routing
    // processing
    auto args = parse_args( argc, argv );

    // create a module loader object at the supplied or default path
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

    // execute the module
    DPMError execute_error = loader.execute_module(args.module_name, args.command);

    // pair result with a message and exit with the appropriate error code
    return print_error( execute_error, args.module_name, args.module_path );
}