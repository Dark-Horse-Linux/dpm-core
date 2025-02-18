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
    if (auto result = loader.check_module_path(); result != DPMError::SUCCESS) {
        switch (result) {
            case DPMError::PATH_NOT_FOUND:
                std::cerr << "Module path not found: " << loader.get_absolute_module_path() << std::endl;
            break;
            case DPMError::PATH_NOT_DIRECTORY:
                std::cerr << "Not a directory: " << loader.get_absolute_module_path() << std::endl;
            break;
            case DPMError::PERMISSION_DENIED:
                std::cerr << "Permission denied: " << loader.get_absolute_module_path() << std::endl;
            break;
            default:
                std::cerr << "Failed checking module path: " << loader.get_absolute_module_path() << std::endl;
        }
        return 1;
    }
    return 0;
}

// list the modules
int main_list_modules(const ModuleLoader& loader)
{
    auto [modules, list_error] = loader.list_available_modules();
    if (list_error != DPMError::SUCCESS) {
        switch (list_error) {
            case DPMError::PERMISSION_DENIED:
                std::cerr << "Permission denied reading modules from: " << loader.get_absolute_module_path() << std::endl;
            break;
            default:
                std::cerr << "Failed listing modules from: " << loader.get_absolute_module_path() << std::endl;
        }
        return 1;
    }

    std::cout << "Available modules in " << loader.get_absolute_module_path() << ":\n";
    for (const auto& module : modules) {
        std::cout << "  " << module << "\n";
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
                std::cout << "Usage: dpm [options] [module-name] [module args...]\n"
                         << "Options:\n"
                         << "  -m, --module-path PATH   Path to DPM modules\n"
                         << "  -h, --help              Show this help message\n"
                         << "\nIf no module is specified, available modules will be listed.\n";
            exit(0);
            case '?':
                exit(1);
        }
    }

    // If there are remaining args, the first one is the module name
    if (optind < argc) {
        args.module_name = argv[optind++];

        // Collect all remaining arguments and combine them into a single command string
        for (int i = optind; i < argc; i++) {
            if (!args.command.empty()) {
                args.command += " ";
            }

            // Handle arguments with spaces by quoting them
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