#include "dpm_interface.hpp"

/*
 *
 *  DPM Interface methods.  These are wrappers of DPM functionality that are meant to handle user view, turning
 *  error codes into human-presentable information, etc.  Features are defined internally, these will only ever be
 *  wrappers of existing features to provide the human/cli interface.
 *
 */

// check if the module path exists
int main_check_module_path(const ModuleLoader& loader)
{
    std::string path;
    DPMError path_error = loader.get_absolute_module_path(path);
    if (path_error != DPMError::SUCCESS) {
        switch (path_error) {
            case DPMError::PATH_NOT_FOUND:
                std::cerr << "Module path not found: " << path << std::endl;
            break;
            case DPMError::PATH_NOT_DIRECTORY:
                std::cerr << "Not a directory: " << path << std::endl;
            break;
            case DPMError::PERMISSION_DENIED:
                std::cerr << "Permission denied: " << path << std::endl;
            break;
            default:
                std::cerr << "Failed checking module path: " << path << std::endl;
        }
        return 1;
    }
    return 0;
}

// list the modules with version information in table format
int main_list_modules(const ModuleLoader& loader)
{
    std::vector<std::string> modules;
    std::string path, abs_path;

    DPMError get_path_error = loader.get_module_path(path);
    if (get_path_error != DPMError::SUCCESS) {
        std::cerr << "Failed to get module path" << std::endl;
        return 1;
    }

    DPMError list_error = loader.list_available_modules(modules);
    if (list_error != DPMError::SUCCESS) {
        loader.get_absolute_module_path(abs_path);
        switch (list_error) {
            case DPMError::PERMISSION_DENIED:
                std::cerr << "Permission denied reading modules from: " << path << std::endl;
            break;
            default:
                std::cerr << "Failed listing modules from: " << path << std::endl;
        }
        return 1;
    }

    if (modules.empty()) {
        std::cout << "No modules found in '" << path << "'." << std::endl;
        return 0;
    }

    std::vector<std::string> valid_modules;
    for (const auto& module : modules) {
        void* handle;
        DPMError load_error = loader.load_module(module, handle);
        if (load_error != DPMError::SUCCESS) {
            continue;
        }

        std::vector<std::string> missing_symbols;
        DPMError validate_error = loader.validate_module_interface(handle, missing_symbols);
        if (validate_error == DPMError::SUCCESS) {
            valid_modules.push_back(module);
        }
        dlclose(handle);
    }

    if (valid_modules.empty()) {
        std::cout << "No valid modules found in '" << path << "'." << std::endl;
        return 0;
    }

    size_t max_name_length = 0;
    size_t max_version_length = 0;
    for (const auto& module : valid_modules) {
        void* module_handle;
        std::string version;
        max_name_length = std::max(max_name_length, module.length());

        DPMError load_error = loader.load_module(module, module_handle);
        if (load_error == DPMError::SUCCESS) {
            DPMError version_error = loader.get_module_version(module_handle, version);
            if (version_error == DPMError::SUCCESS) {
                max_version_length = std::max(max_version_length, version.length());
            }
            dlclose(module_handle);
        }
    }

    const int column_spacing = 4;

    std::cout << "Available modules in '" << path << "':" << std::endl << std::endl;
    std::cout << std::left << std::setw(max_name_length + column_spacing) << "MODULE"
              << std::setw(max_version_length + column_spacing) << "VERSION"
              << "DESCRIPTION" << std::endl;

    for (const auto& module_name : valid_modules) {
        void* module_handle;
        std::string version = "unknown";
        std::string description = "unknown";

        DPMError load_error = loader.load_module(module_name, module_handle);
        if (load_error == DPMError::SUCCESS) {
            DPMError version_error = loader.get_module_version(module_handle, version);
            DPMError desc_error = loader.get_module_description(module_handle, description);
            dlclose(module_handle);
        }

        std::cout << std::left << std::setw(max_name_length + column_spacing) << module_name
                  << std::setw(max_version_length + column_spacing) << version
                  << description << std::endl;
    }

    return 0;
}


// parser for populating data structure for supplied arguments
CommandArgs parse_args(int argc, char* argv[])
{
    CommandArgs args;

    static struct option long_options[] = {
        {"module-path", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                args.module_path = optarg;
            break;
            case 'h':
                std::cout << "Usage: dpm [options] [module-name] [module args...]\n\n"
                         << "Options:\n\n"
                         << "  -m, --module-path PATH   Path to DPM modules\n"
                         << "  -h, --help              Show this help message\n"
                         << "\nIf no module is specified, available modules will be listed.\n\n";
            exit(0);
            case '?':
                exit(1);
        }
    }

    if (optind < argc) {
        args.module_name = argv[optind++];

        for (int i = optind; i < argc; i++) {
            if (!args.command.empty()) {
                args.command += " ";
            }

            std::string arg = argv[i];
            if (arg.find(' ') != std::string::npos) {
                args.command += "\"" + arg + "\"";
            } else {
                args.command += arg;
            }
        }
    }

    return args;
}