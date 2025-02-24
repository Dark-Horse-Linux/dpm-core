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
    loader.get_module_path(path);

    if (!std::filesystem::exists(path)) {
        std::cerr << "Module path does not exist: " << path << std::endl;
        return 1;
    }

    if (!std::filesystem::is_directory(path)) {
        std::cerr << "Module path is not a directory: " << path << std::endl;
        return 1;
    }

    try {
        auto perms = std::filesystem::status(path).permissions();
        if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
            std::cerr << "Permission denied: " << path << std::endl;
            return 1;
        }
    } catch (const std::filesystem::filesystem_error&) {
        std::cerr << "Permission denied: " << path << std::endl;
        return 1;
    }

    return 0;
}

// list the modules
int main_list_modules(const ModuleLoader& loader)
{
    std::vector<std::string> modules;
    std::string path;

    DPMError get_path_error = loader.get_module_path(path);
    if (get_path_error != DPMError::SUCCESS) {
        std::cerr << "Failed to get module path" << std::endl;
        return 1;
    }

    DPMError list_error = loader.list_available_modules(modules);
    if (list_error != DPMError::SUCCESS) {
        std::cerr << "No modules found in: " << path << std::endl;
        return 1;
    }

    if (modules.empty()) {
        std::cout << "No modules found in '" << path << "'." << std::endl;
        return 0;
    }

    std::vector<std::string> valid_modules;
    for (int i = 0; i < modules.size(); i++) {
        void* handle;
        DPMError load_error = loader.load_module(modules[i], handle);
        if (load_error != DPMError::SUCCESS) {
            continue;
        }

        std::vector<std::string> missing_symbols;
        DPMError validate_error = loader.validate_module_interface(handle, missing_symbols);
        if (validate_error == DPMError::SUCCESS) {
            valid_modules.push_back(modules[i]);
        }
        dlclose(handle);
    }

    if (valid_modules.empty()) {
        std::cout << "No valid modules found in '" << path << "'." << std::endl;
        return 0;
    }

    size_t max_name_length = 0;
    size_t max_version_length = 0;
    for (int i = 0; i < valid_modules.size(); i++) {
        void* module_handle;
        std::string version;
        max_name_length = std::max(max_name_length, valid_modules[i].length());

        DPMError load_error = loader.load_module(valid_modules[i], module_handle);
        if (load_error == DPMError::SUCCESS) {
            DPMError version_error = loader.get_module_version(module_handle, version);
            if (version_error == DPMError::SUCCESS) {
                max_version_length = std::max(max_version_length, version.length());
            }
            dlclose(module_handle);
        }
    }

    const int column_spacing = 4;

    std::cout << "\nAvailable modules in '" << path << "':" << std::endl << std::endl;
    std::cout << std::left << std::setw(max_name_length + column_spacing) << "MODULE"
              << std::setw(max_version_length + column_spacing) << "VERSION"
              << "DESCRIPTION" << std::endl;

    for (int i = 0; i < valid_modules.size(); i++) {
        void* module_handle;
        std::string version = "unknown";
        std::string description = "unknown";

        DPMError load_error = loader.load_module(valid_modules[i], module_handle);
        if (load_error == DPMError::SUCCESS) {
            DPMError version_error = loader.get_module_version(module_handle, version);
            DPMError desc_error = loader.get_module_description(module_handle, description);
            dlclose(module_handle);
        }

        std::cout << std::left << std::setw(max_name_length + column_spacing) << valid_modules[i]
                  << std::setw(max_version_length + column_spacing) << version
                  << description << std::endl;
    }

    return 0;
}

CommandArgs parse_args(int argc, char* argv[])
{
    CommandArgs args;
    args.module_path = "/usr/lib/dpm/modules/";  // Set to same default as ModuleLoader

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
                          << "  -h, --help              Show this help message\n\n"
                          << "If no module is specified, available modules will be listed.\n\n";
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
        case DPMError::MODULE_NOT_LOADED:
            std::cerr << "Attempted to execute module before loading it: " << module_name << std::endl;
        return 1;
        case DPMError::MODULE_LOAD_FAILED:
            std::cerr << "Failed to load module: " << module_name << std::endl;
        return 1;
        case DPMError::INVALID_MODULE:
            std::cerr << "Invalid module format: " << module_name << std::endl;
        return 1;
        case DPMError::UNDEFINED_ERROR:
            std::cerr << "Undefined error occurred with module: " << module_name << std::endl;
        return 1;
        default:
            std::cerr << "Unknown error executing module: " << module_name << std::endl;
        return 1;
    }
}