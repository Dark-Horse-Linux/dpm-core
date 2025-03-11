#include "cli_parsers.hpp"

int parse_create_options(int argc, char** argv, BuildOptions& options) {
    // For debugging
    dpm_log(LOG_DEBUG, "Parsing command-line arguments");
    for (int i = 0; i < argc; i++) {
        std::string arg_msg = "Arg " + std::to_string(i) + ": " + (argv[i] ? argv[i] : "(null)");
        dpm_log(LOG_DEBUG, arg_msg.c_str());
    }

    // First process any arguments in --option=value format
    for (int i = 0; i < argc; i++) {
        if (!argv[i] || strlen(argv[i]) == 0) continue;

        std::string arg(argv[i]);
        size_t equals_pos = arg.find('=');

        if (equals_pos != std::string::npos && arg.length() > 3 && arg[0] == '-' && arg[1] == '-') {
            // Extract option and value
            std::string option = arg.substr(0, equals_pos);
            std::string value = arg.substr(equals_pos + 1);

            if (option == "--output-dir") {
                options.output_dir = value;
            } else if (option == "--contents") {
                options.contents_dir = value;
            } else if (option == "--metadata") {
                options.metadata_dir = value;
            } else if (option == "--hooks") {
                options.hooks_dir = value;
            } else if (option == "--name") {
                options.package_name = value;
            } else if (option == "--sign") {
                options.signature_key = value;
            } else if (option == "--force") {
                options.force = true;
            } else if (option == "--verbose") {
                options.verbose = true;
            } else if (option == "--help") {
                options.show_help = true;
            }

            // Convert this argument to a dummy to prevent getopt from processing it
            argv[i] = strdup("--dummy");
        }
    }

    static struct option long_options[] = {
        {"output-dir", required_argument, 0, 'o'},
        {"contents", required_argument, 0, 'c'},
        {"metadata", required_argument, 0, 'm'},
        {"hooks", required_argument, 0, 'H'},
        {"name", required_argument, 0, 'n'},
        {"sign", required_argument, 0, 's'},
        {"force", no_argument, 0, 'f'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"dummy", no_argument, 0, 0},  // Add dummy option to prevent getopt errors
        {0, 0, 0, 0}
    };

    // Reset getopt
    optind = 0;
    opterr = 0;  // Suppress getopt error messages

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "o:c:m:H:n:s:fvh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                options.output_dir = optarg;
                break;
            case 'c':
                options.contents_dir = optarg;
                break;
            case 'm':
                options.metadata_dir = optarg;
                break;
            case 'H':
                options.hooks_dir = optarg;
                break;
            case 'n':
                options.package_name = optarg;
                break;
            case 's':
                options.signature_key = optarg;
                break;
            case 'f':
                options.force = true;
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'h':
                options.show_help = true;
                break;
            case '?':
                // Ignore errors as we handle equals-format options separately
                break;
            default:
                break;
        }
    }

    // Expand paths after all arguments have been processed
    if (!options.output_dir.empty()) {
        options.output_dir = expand_path(options.output_dir);
    }

    if (!options.contents_dir.empty()) {
        options.contents_dir = expand_path(options.contents_dir);
    }

    if (!options.metadata_dir.empty()) {
        options.metadata_dir = expand_path(options.metadata_dir);
    }

    if (!options.hooks_dir.empty()) {
        options.hooks_dir = expand_path(options.hooks_dir);
    }

    if (!options.signature_key.empty()) {
        options.signature_key = expand_path(options.signature_key);
    }

    // Log the parsed options for debugging
    dpm_log(LOG_DEBUG, ("Parsed options: contents_dir=" + options.contents_dir).c_str());

    return 0;
}

Command parse_command(const char* cmd_str) {
    if (cmd_str == nullptr || strlen(cmd_str) == 0) {
        return CMD_HELP;
    }

    if (strcmp(cmd_str, "help") == 0) {
        return CMD_HELP;
    }
    else if (strcmp(cmd_str, "create") == 0) {
        return CMD_CREATE;
    }

    return CMD_UNKNOWN;
}

int validate_build_options(const BuildOptions& options) {
    // Check if contents directory is provided and exists
    if (options.contents_dir.empty()) {
        dpm_log(LOG_ERROR, "Contents directory is required (--contents)");
        return 1;
    }

    if (!std::filesystem::exists(options.contents_dir)) {
        dpm_log(LOG_ERROR, ("Contents directory does not exist: " + options.contents_dir).c_str());
        return 1;
    }

    // Check if metadata directory is provided and exists
    if (options.metadata_dir.empty()) {
        dpm_log(LOG_ERROR, "Metadata directory is required (--metadata)");
        return 1;
    }

    if (!std::filesystem::exists(options.metadata_dir)) {
        dpm_log(LOG_ERROR, ("Metadata directory does not exist: " + options.metadata_dir).c_str());
        return 1;
    }

    // Check if hooks directory exists if provided
    if (!options.hooks_dir.empty() && !std::filesystem::exists(options.hooks_dir)) {
        dpm_log(LOG_ERROR, ("Hooks directory does not exist: " + options.hooks_dir).c_str());
        return 1;
    }

    // Check if output directory exists
    if (!std::filesystem::exists(options.output_dir)) {
        dpm_log(LOG_ERROR, ("Output directory does not exist: " + options.output_dir).c_str());
        return 1;
    }

    // Check if signature key exists if provided
    if (!options.signature_key.empty() && !std::filesystem::exists(options.signature_key)) {
        dpm_log(LOG_ERROR, ("Signature key file does not exist: " + options.signature_key).c_str());
        return 1;
    }

    return 0;
}
