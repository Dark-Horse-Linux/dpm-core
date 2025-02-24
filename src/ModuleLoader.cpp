#include "ModuleLoader.hpp"

namespace fs = std::filesystem;

ModuleLoader::ModuleLoader(std::string module_path)
{
    try {
        _module_path = fs::absolute(module_path).string();
        if (!_module_path.empty() && _module_path.back() != '/') {
            _module_path += '/';
        }
    } catch (const fs::filesystem_error&) {
        _module_path = module_path;
        if (!_module_path.empty() && _module_path.back() != '/') {
            _module_path += '/';
        }
    }
}

DPMError ModuleLoader::get_module_path(std::string& path) const
{
    path = _module_path;
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::list_available_modules(std::vector<std::string>& modules) const
{
    modules.clear();

    try {
        for (const auto& entry : fs::directory_iterator(_module_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".so") {
                    modules.push_back(filename.substr(0, filename.size() - 3));
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        return DPMError::PERMISSION_DENIED;
    }

    return DPMError::SUCCESS;
}

DPMError ModuleLoader::load_module(const std::string& module_name, void*& module_handle) const
{
    // construct the path to load the module from based on supplied identifier
    // DPM uses whatever the file name is
    std::string module_so_path = _module_path + module_name + ".so";


    module_handle = dlopen(module_so_path.c_str(), RTLD_LAZY);
    if ( !module_handle ) {
        return DPMError::MODULE_LOAD_FAILED;
    }

    const char * load_error = dlerror();
    if ( load_error != nullptr ) {
        return DPMError::MODULE_LOAD_FAILED;
    }

    std::vector<std::string> missing_symbols;
    DPMError validate_error = validate_module_interface(module_handle, missing_symbols);
    if ( validate_error != DPMError::SUCCESS ) {
        dlclose(module_handle);
        return validate_error;
    }

    return validate_error;
}

DPMError ModuleLoader::execute_module( const std::string& module_name, const std::string& command ) const
{
    // declare a module_handle
    void * module_handle;

    // attempt to load the module
    DPMError load_error = load_module( module_name, module_handle );
    if ( load_error != DPMError::SUCCESS ) {
        return load_error;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if ( pre_error != nullptr ) {
        dlclose( module_handle );
        return DPMError::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef int (*ExecuteFn) ( const char*, int, char** );

    // populate that void pointer to the execute symbol in the module with
    ExecuteFn execute_fn = (ExecuteFn) dlsym( module_handle, "dpm_module_execute" );

    // do basic error handling to detect if the symbol look up was successful
    const char * dlsym_error = dlerror();
    if ( dlsym_error != nullptr ) {
        dlclose( module_handle );
        return DPMError::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if ( execute_fn == nullptr ) {
        dlclose( module_handle );
        return DPMError::SYMBOL_NOT_FOUND;
    }

    // execute the symbol that was loaded and supply the command string being routed from DPM
    int exec_error = execute_fn( command.c_str(), 0, nullptr );

    // irregardless of result, this is the time to close the module handle
    dlclose( module_handle );

    // if the result of execution was not 0, return an error
    if ( exec_error != 0 ) {
        return DPMError::SYMBOL_EXECUTION_FAILED;
    }

    // if we made it here, assume it was successful
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::get_module_version( void * module_handle, std::string& version ) const
{
    // validate that the module is even loaded
    if ( !module_handle ) {
        version = "DPM ERROR";
        return DPMError::MODULE_NOT_LOADED;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if ( pre_error != nullptr ) {
        version = pre_error;
        return DPMError::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef const char * (* VersionFn)();

    // populate that void pointer to execute the symbol in the module with
    VersionFn version_fn = (VersionFn) dlsym( module_handle, "dpm_module_get_version" );

    // Check for errors from dlsym
    const char* error = dlerror();
    if (error != nullptr) {
        version = error;
        return DPMError::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if ( version_fn == nullptr ) {
        version = "ERROR";
        return DPMError::SYMBOL_NOT_FOUND;
    }

    // execute the loaded symbol
    const char * ver = version_fn();

    // check the return, and throw an error if it's a null value
    if ( ver == nullptr ) {
        version = "MODULE ERROR";
        return DPMError::INVALID_MODULE;
    }

    // if you made it here, assume success
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::get_module_description( void * module_handle, std::string& description ) const
{
    // validate that the module is even loaded
    if (!module_handle) {
        description = "DPM ERROR";
        return DPMError::MODULE_NOT_LOADED;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if ( pre_error != nullptr ) {
        description = pre_error;
        return DPMError::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef const char * (* DescriptionFn)();

    // populate that void pointer to execute the symbol in the module with
    DescriptionFn description_fn = (DescriptionFn) dlsym( module_handle, "dpm_get_description" );

    // check for errors from dlsym
    const char* error = dlerror();
    if ( error != nullptr ) {
        description = "ERROR";
        return DPMError::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if ( description_fn == nullptr ) {
        description = "ERROR";
        return DPMError::INVALID_MODULE;
    }

    // execute the loaded symbol
    const char* desc = description_fn();

    // check the return, and throw an error if it's a null value
    if ( desc == nullptr ) {
        description = "MODULE ERROR";
        return DPMError::INVALID_MODULE;
    }

    // if you made it here, assume success
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::validate_module_interface( void* module_handle, std::vector<std::string>& missing_symbols ) const
{
    // validate that the module is even loaded
    if ( !module_handle ) {
        return DPMError::MODULE_NOT_LOADED;
    }

    // ensure our starting point of missing symbols is empty
    missing_symbols.clear();

    // get the size of the loop (should be equal to the number of required symbols)
    size_t num_symbols = module_interface::required_symbols.size();

    // check for any residual lingering errors
    const char * pre_error = dlerror();
    if ( pre_error != nullptr ) {
        return DPMError::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef const char * (* ModuleInterfaceFn)();

    // iterate through self.required_symbols
    for ( size_t i = 0; i < num_symbols; i++ ) {

        // attempt to load each required symbol
        ModuleInterfaceFn loaded_required_symbol = (ModuleInterfaceFn) dlsym( module_handle, module_interface::required_symbols[i].c_str() );

        // check for an error from dlsym
        const char * dlsym_error = dlerror();
        if (dlsym_error != nullptr) {
            // if nullptr, it didn't load, so assume it's missing
            missing_symbols.push_back(module_interface::required_symbols[i]);
        }

    }

    // if there are no missing symbols, return successfully -- the module has a valid API
    if ( missing_symbols.empty() ) {
        return DPMError::SUCCESS;
    }

    // if not successful, the module's API is invalid and return the appropriate error code
    return DPMError::INVALID_MODULE;
}
