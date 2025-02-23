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

    // create a module handle
    void * module_handle;

    // load the user-supplied module to execute
    DPMError load_error = loader.load_module( args.module_name, module_handle );

    // if that failed, additionally print an error and return a non-zero exit code
    // TODO: verify that loader.load_module is actually doing error handling
    if ( load_error != DPMError::SUCCESS ) {
        std::cerr << "Failed to load module: " << args.module_name << std::endl;
        return 1;
    }

    // execute the module and provide the user-supplied command to execute
    DPMError execute_error = loader.execute_module( module_handle, args.command );

    // there is no retry logic, so, whether execute succeeded
    // or failed, clean up the module handle
    dlclose(module_handle);

    // check the execution result and if it failed, report an additional error
    // TODO: verify that loader.execute_module is actually doing error handling
    if (execute_error != DPMError::SUCCESS) {
        std::cerr << "Failed to execute module: " << args.module_name << std::endl;
        return 1;
    }

    return 0;
}