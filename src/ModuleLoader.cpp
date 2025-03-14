/**
 * @file ModuleLoader.cpp
 * @brief Implementation of dynamic module loading and management for DPM
 *
 * Implements the ModuleLoader class methods for finding, loading, validating,
 * and executing DPM modules. It handles the dynamic loading of shared objects
 * and ensures they conform to the expected module interface.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * For bug reports or contributions, please contact the dhlp-contributors
 * mailing list at: https://lists.darkhorselinux.org/mailman/listinfo/dhlp-contributors
 */

#include "ModuleLoader.hpp"

namespace fs = std::filesystem;

// ModuleLoader constructor
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

// getter for module path
DPMErrorCategory ModuleLoader::get_module_path(std::string& path) const
{
    path = _module_path;
    return DPMErrorCategory::SUCCESS;
}

// Helper method to check module path validity
DPMErrorCategory ModuleLoader::check_module_path() const
{
    // Verify the path exists and get its properties
    struct stat path_stat;
    if (stat(_module_path.c_str(), &path_stat) != 0) {
        // Check errno to determine the specific error
        switch (errno) {
            case ENOENT:
                return DPMErrorCategory::PATH_NOT_FOUND;
            case EACCES:
                return DPMErrorCategory::PERMISSION_DENIED;
            case ENAMETOOLONG:
                return DPMErrorCategory::PATH_TOO_LONG;
            case ENOTDIR:
                // This happens when a component of the path prefix isn't a directory
                    return DPMErrorCategory::PATH_NOT_DIRECTORY;
            default:
                return DPMErrorCategory::UNDEFINED_ERROR;
        }
    }

    // At this point stat() succeeded, now check if the final path component is a directory
    if (!S_ISDIR(path_stat.st_mode)) {
        return DPMErrorCategory::PATH_NOT_DIRECTORY;
    }

    // Check read permissions using the stat results
    if ((path_stat.st_mode & S_IRUSR) == 0) {
        return DPMErrorCategory::PERMISSION_DENIED;
    }

    return DPMErrorCategory::SUCCESS;
}

DPMErrorCategory ModuleLoader::list_available_modules(std::vector<std::string>& modules) const
{
    // ensure we start with an empty vector
    modules.clear();

    // Check module path using the helper method
    DPMErrorCategory path_check = check_module_path();
    if (path_check != DPMErrorCategory::SUCCESS) {
        return path_check;
    }

    // prepare to iterate the directory
    DIR* dir = opendir(_module_path.c_str());
    if (!dir) {
        // Check errno to determine the cause of the failure
        switch (errno) {
            case EACCES:
                return DPMErrorCategory::PERMISSION_DENIED;
            case ENOENT:
                return DPMErrorCategory::PATH_NOT_FOUND;
            case ENOTDIR:
                return DPMErrorCategory::PATH_NOT_DIRECTORY;
            default:
                return DPMErrorCategory::UNDEFINED_ERROR;
        }
    }

    // read each entry
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // build the full path
        std::string full_path = _module_path + entry->d_name;

        // verify it's a file or a symlink
        struct stat st;
        if (stat(full_path.c_str(), &st) == -1) {
            continue;
        }

        // Skip if not a regular file or a symlink
        if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
            continue;
        }

        // get length of filename for boundary checking
        size_t name_len = strlen(entry->d_name);

        // skip if filename too short to be .so
        if (name_len <= 3) {
            continue;
        }

        // verify file ends in .so
        if (strcmp(entry->d_name + name_len - 3, ".so") != 0) {
            continue;
        }

        // calculate the module name length
        size_t module_name_len = name_len - 3;

        // create space for the module name
        char * module_name = (char*) malloc(module_name_len + 1);
        if (!module_name) {
            closedir(dir);
            return DPMErrorCategory::UNDEFINED_ERROR;
        }

        // copy the name without .so
        strncpy(module_name, entry->d_name, module_name_len);
        module_name[module_name_len] = '\0';

        // add to our list
        modules.push_back(std::string(module_name));

        // clean up
        free(module_name);
    }

    // clean up
    closedir(dir);

    return DPMErrorCategory::SUCCESS;
}

DPMErrorCategory ModuleLoader::load_module(const std::string& module_name, void*& module_handle) const
{
    // First check if the module exists in the list of available modules
    std::vector<std::string> available_modules;
    DPMErrorCategory list_error = list_available_modules( available_modules );

    // if there was a listing error, return that
    if ( list_error != DPMErrorCategory::SUCCESS ) {
        return list_error;
    }

    // Check if the requested module is in the list of available modules
    bool module_found = false;
    for ( size_t i = 0; i < available_modules.size(); i++ ) {
        if ( available_modules[i] == module_name ) {
            module_found = true;
            break;
        }
    }

    // if the supplied module isn't in the list of available modules, return the error
    if ( !module_found ) {
        return DPMErrorCategory::MODULE_NOT_FOUND;
    }

    // construct the path to load the module from based on supplied identifier
    // DPM uses whatever the file name is
    std::string module_so_path = _module_path + module_name + ".so";

    // go ahead and open the module
    module_handle = dlopen(module_so_path.c_str(), RTLD_LAZY);
    if (!module_handle) {
        return DPMErrorCategory::MODULE_LOAD_FAILED;
    }

    // if there was a loading error, return that
    const char * load_error = dlerror();
    if ( load_error != nullptr ) {
        return DPMErrorCategory::MODULE_LOAD_FAILED;
    }

    // validate the module's exposed API
    // return an error if it's not up to spec
    std::vector<std::string> missing_symbols;
    DPMErrorCategory validate_error = validate_module_interface( module_handle, missing_symbols );
    if ( validate_error != DPMErrorCategory::SUCCESS ) {
        // we failed to validate the interface, so close the module handle since we won't use it
        dlclose( module_handle );
        return validate_error;
    }

    // return what's going to be a success
    return validate_error;
}

DPMErrorCategory ModuleLoader::execute_module(const std::string& module_name, const std::string& command) const {
    // declare a module_handle
    void * module_handle;

    // attempt to load the module
    DPMErrorCategory load_error = load_module(module_name, module_handle);
    if (load_error != DPMErrorCategory::SUCCESS) {
        return load_error;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if (pre_error != nullptr) {
        dlclose(module_handle);
        return DPMErrorCategory::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef int (*ExecuteFn) (const char*, int, char** );

    // populate that void pointer to the execute symbol in the module with
    ExecuteFn execute_fn = (ExecuteFn) dlsym(module_handle, "dpm_module_execute");

    // do basic error handling to detect if the symbol look up was successful
    const char * dlsym_error = dlerror();
    if (dlsym_error != nullptr) {
        dlclose(module_handle);
        return DPMErrorCategory::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if (execute_fn == nullptr) {
        dlclose(module_handle);
        return DPMErrorCategory::SYMBOL_NOT_FOUND;
    }

    // Split the command by spaces to get arguments
    std::vector<std::string> args;
    std::string arg;
    std::istringstream iss(command);
    while (iss >> arg) {
        args.push_back(arg);
    }

    // Create argc and argv for all arguments
    int argc = args.size();
    char** argv = new char*[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = strdup(args[i].c_str());
    }

    // Get the first argument as the command (or empty string if no arguments)
    const char* cmd = "";
    if (!args.empty()) {
        cmd = args[0].c_str();
    }

    // execute the function with all arguments
    int exec_error = execute_fn(cmd, argc, argv);

    // Clean up
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    delete[] argv;

    // irregardless of result, this is the time to close the module handle
    dlclose(module_handle);

    // if the result of execution was not 0, return an error
    if (exec_error != 0) {
        return DPMErrorCategory::SYMBOL_EXECUTION_FAILED;
    }

    // if we made it here, assume it was successful
    return DPMErrorCategory::SUCCESS;
}

DPMErrorCategory ModuleLoader::get_module_version( void * module_handle, std::string& version ) const
{
    // validate that the module is even loaded
    if ( !module_handle ) {
        version = "DPM ERROR";
        return DPMErrorCategory::MODULE_NOT_LOADED;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if ( pre_error != nullptr ) {
        version = pre_error;
        return DPMErrorCategory::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef const char * (* VersionFn)();

    // populate that void pointer to execute the symbol in the module with
    VersionFn version_fn = (VersionFn) dlsym( module_handle, "dpm_module_get_version" );

    // Check for errors from dlsym
    const char* error = dlerror();
    if (error != nullptr) {
        version = error;
        return DPMErrorCategory::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if ( version_fn == nullptr ) {
        version = "ERROR";
        return DPMErrorCategory::SYMBOL_NOT_FOUND;
    }

    // execute the loaded symbol
    const char * ver = version_fn();

    // check the return, and throw an error if it's a null value
    if ( ver == nullptr ) {
        version = "MODULE ERROR";
        return DPMErrorCategory::INVALID_MODULE;
    }

    // Set the version string with the result
    version = ver;

    // if you made it here, assume success
    return DPMErrorCategory::SUCCESS;
}

DPMErrorCategory ModuleLoader::get_module_description( void * module_handle, std::string& description ) const
{
    // validate that the module is even loaded
    if (!module_handle) {
        description = "DPM ERROR";
        return DPMErrorCategory::MODULE_NOT_LOADED;
    }

    // Clear any previous error state and handle any residual failure
    const char* pre_error = dlerror();
    if ( pre_error != nullptr ) {
        description = pre_error;
        return DPMErrorCategory::UNDEFINED_ERROR;
    }

    // declare a function pointer type to hold the module symbol to execute
    typedef const char * (* DescriptionFn)();

    // populate that void pointer to execute the symbol in the module with
    DescriptionFn description_fn = (DescriptionFn) dlsym( module_handle, "dpm_get_description" );

    // check for errors from dlsym
    const char * error = dlerror();
    if ( error != nullptr ) {
        description = "ERROR";
        return DPMErrorCategory::SYMBOL_NOT_FOUND;
    }

    // check if the void pointer was populated
    if ( description_fn == nullptr ) {
        description = "ERROR";
        return DPMErrorCategory::INVALID_MODULE;
    }

    // execute the loaded symbol
    const char* desc = description_fn();

    // check the return, and throw an error if it's a null value
    if ( desc == nullptr ) {
        description = "MODULE ERROR";
        return DPMErrorCategory::INVALID_MODULE;
    }

    // Set the description string with the result
    description = desc;

    // if you made it here, assume success
    return DPMErrorCategory::SUCCESS;
}

DPMErrorCategory ModuleLoader::validate_module_interface( void* module_handle, std::vector<std::string>& missing_symbols ) const
{
    // validate that the module is even loaded
    if ( !module_handle ) {
        return DPMErrorCategory::MODULE_NOT_LOADED;
    }

    // ensure our starting point of missing symbols is empty
    missing_symbols.clear();

    // get the size of the loop (should be equal to the number of required symbols)
    size_t num_symbols = module_interface::required_symbols.size();

    // check for any residual lingering errors
    const char * pre_error = dlerror();
    if ( pre_error != nullptr ) {
        return DPMErrorCategory::UNDEFINED_ERROR;
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
        return DPMErrorCategory::SUCCESS;
    }

    // if not successful, the module's API is invalid and return the appropriate error code
    return DPMErrorCategory::INVALID_MODULE;
}
