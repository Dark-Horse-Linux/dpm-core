#include "dpm_interface_helpers.hpp"

/**
 * Parse command line arguments for DPM.
 *
 * This function parses the command line arguments provided to DPM
 * and builds a CommandArgs structure containing the parsed values.
 *
 * @param argc The number of arguments provided to the program
 * @param argv Array of C-style strings containing the arguments
 *
 * @return CommandArgs structure containing the parsed command line arguments
 *
 * The function handles the following arguments:
 * - ``-m, --module-path PATH``: Sets the directory path where DPM modules are located
 * - ``-h, --help``: Displays a help message and exits
 *
 * Additional arguments are processed as follows:
 * - First non-option argument is treated as the module name
 * - All remaining arguments are combined into a single command string for the module
 *
 * If the argument contains spaces, it will be quoted in the command string.
 *
 * If no module name is provided, the module_name field will be empty.
 */
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