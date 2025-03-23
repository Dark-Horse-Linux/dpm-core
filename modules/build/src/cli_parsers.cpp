#include "cli_parsers.hpp"

int parse_create_options(int argc, char** argv, BuildOptions& options) {
    // Check for help flags directly before any other parsing
    for (int i = 0; i < argc; i++) {
        if (argv[i] && (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "help") == 0)) {
            options.show_help = true;
            // Skip all other parsing for help flag
            return 0;
        }
    }

    // Track which options were explicitly provided on command line
    bool output_dir_provided = false;
    bool contents_dir_provided = false;
    bool hooks_dir_provided = false;
    bool package_name_provided = false;
    bool package_version_provided = false;
    bool architecture_provided = false;
    bool os_provided = false;
    bool force_provided = false;
    bool verbose_provided = false;
    bool help_provided = false;

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

            if (option == "--output") {
                options.output_dir = value;
                output_dir_provided = true;
            } else if (option == "--contents") {
                options.contents_dir = value;
                contents_dir_provided = true;
            } else if (option == "--hooks") {
                options.hooks_dir = value;
                hooks_dir_provided = true;
            } else if (option == "--name") {
                options.package_name = value;
                package_name_provided = true;
            } else if (option == "--version") {
                options.package_version = value;
                package_version_provided = true;
            } else if (option == "--architecture") {
                options.architecture = value;
                architecture_provided = true;
            } else if (option == "--os") {
                options.os = value;
                os_provided = true;
            } else if (option == "--force") {
                // Parse the boolean value
                options.force = (value == "true" || value == "1" || value == "yes");
                force_provided = true;
            } else if (option == "--verbose") {
                // Parse the boolean value
                options.verbose = (value == "true" || value == "1" || value == "yes");
                verbose_provided = true;
            } else if (option == "--help") {
                // Parse the boolean value
                options.show_help = (value == "true" || value == "1" || value == "yes");
                help_provided = true;
            }

            // Convert this argument to a dummy to prevent getopt from processing it
            argv[i] = strdup("--dummy");
        }
    }

    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"contents", required_argument, 0, 'c'},
        {"hooks", required_argument, 0, 'H'},
        {"name", required_argument, 0, 'n'},
        {"version", required_argument, 0, 'V'},
        {"architecture", required_argument, 0, 'a'},
        {"os", required_argument, 0, 'O'},
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

    while ((opt = getopt_long(argc, argv, "o:c:H:n:V:a:O:fvh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                options.output_dir = optarg;
                output_dir_provided = true;
                break;
            case 'c':
                options.contents_dir = optarg;
                contents_dir_provided = true;
                break;
            case 'H':
                options.hooks_dir = optarg;
                hooks_dir_provided = true;
                break;
            case 'n':
                options.package_name = optarg;
                package_name_provided = true;
                break;
            case 'V':
                options.package_version = optarg;
                package_version_provided = true;
                break;
            case 'a':
                options.architecture = optarg;
                architecture_provided = true;
                break;
            case 'O':
                options.os = optarg;
                os_provided = true;
                break;
            case 'f':
                options.force = true;
                force_provided = true;
                break;
            case 'v':
                options.verbose = true;
                verbose_provided = true;
                break;
            case 'h':
                options.show_help = true;
                help_provided = true;
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

    if (!options.hooks_dir.empty()) {
        options.hooks_dir = expand_path(options.hooks_dir);
    }

    // Log the parsed options - only include options explicitly provided by the user
    dpm_log(LOG_DEBUG, "Parsed options:");
    bool any_options_provided = false;

    if (output_dir_provided) {
        dpm_log(LOG_DEBUG, ("  output_dir=" + options.output_dir).c_str());
        any_options_provided = true;
    }

    if (contents_dir_provided) {
        dpm_log(LOG_DEBUG, ("  contents_dir=" + options.contents_dir).c_str());
        any_options_provided = true;
    }

    if (hooks_dir_provided) {
        dpm_log(LOG_DEBUG, ("  hooks_dir=" + options.hooks_dir).c_str());
        any_options_provided = true;
    }

    if (package_name_provided) {
        dpm_log(LOG_DEBUG, ("  package_name=" + options.package_name).c_str());
        any_options_provided = true;
    }

    if (package_version_provided) {
        dpm_log(LOG_DEBUG, ("  package_version=" + options.package_version).c_str());
        any_options_provided = true;
    }

    if (architecture_provided) {
        dpm_log(LOG_DEBUG, ("  architecture=" + options.architecture).c_str());
        any_options_provided = true;
    }

    if (os_provided) {
        dpm_log(LOG_DEBUG, ("  os=" + options.os).c_str());
        any_options_provided = true;
    }

    if (force_provided) {
        dpm_log(LOG_DEBUG, ("  force=" + std::string(options.force ? "true" : "false")).c_str());
        any_options_provided = true;
    }

    if (verbose_provided) {
        dpm_log(LOG_DEBUG, ("  verbose=" + std::string(options.verbose ? "true" : "false")).c_str());
        any_options_provided = true;
    }

    if (help_provided) {
        dpm_log(LOG_DEBUG, ("  help=" + std::string(options.show_help ? "true" : "false")).c_str());
        any_options_provided = true;
    }

    if (!any_options_provided) {
        dpm_log(LOG_DEBUG, "  No options provided - using defaults");
    }

    return 0;
}

Command parse_command(const char* cmd_str) {
    if (cmd_str == nullptr || strlen(cmd_str) == 0) {
        return CMD_HELP;
    }

    // Check for stage command, including when it has additional arguments
    if (strncmp(cmd_str, "stage", 5) == 0) {
        return CMD_STAGE;
    }

    // Check for manifest command, including when it has additional arguments
    if (strncmp(cmd_str, "manifest", 8) == 0) {
        return CMD_MANIFEST;
    }

    // Check for sign command, including when it has additional arguments
    if (strncmp(cmd_str, "sign", 4) == 0) {
        return CMD_SIGN;
    }

    // Check for seal command, including when it has additional arguments
    if (strncmp(cmd_str, "seal", 4) == 0) {
        return CMD_SEAL;
    }

    // Check for unseal command, including when it has additional arguments
    if (strncmp(cmd_str, "unseal", 6) == 0) {
        return CMD_UNSEAL;
    }

    // Check if cmd_str is a help option
    if (strcmp(cmd_str, "-h") == 0 || strcmp(cmd_str, "--help") == 0) {
        return CMD_HELP;
    }
    else if (strcmp(cmd_str, "help") == 0) {
        return CMD_HELP;
    }

    return CMD_UNKNOWN;
}

int validate_build_options(const BuildOptions& options) {
    // Check if help was requested - skip validation in this case
    if (options.show_help) {
        return 0;
    }

    // Check if output directory is provided
    if (options.output_dir.empty()) {
        dpm_log(LOG_ERROR, "Output directory is required (--output)");
        return 1;
    }

    // Check if output directory exists
    if (!std::filesystem::exists(options.output_dir)) {
        dpm_log(LOG_ERROR, ("Output directory does not exist: " + options.output_dir).c_str());
        return 1;
    }

    // Check if contents directory is provided and exists
    if (options.contents_dir.empty()) {
        dpm_log(LOG_ERROR, "Contents directory is required (--contents)");
        return 1;
    }

    if (!std::filesystem::exists(options.contents_dir)) {
        dpm_log(LOG_ERROR, ("Contents directory does not exist: " + options.contents_dir).c_str());
        return 1;
    }

    // Check if package name is provided
    if (options.package_name.empty()) {
        dpm_log(LOG_ERROR, "Package name is required (--name)");
        return 1;
    }

    // Check if package version is provided
    if (options.package_version.empty()) {
        dpm_log(LOG_ERROR, "Package version is required (--version)");
        return 1;
    }

    // Check if architecture is provided
    if (options.architecture.empty()) {
        dpm_log(LOG_ERROR, "Architecture is required (--architecture)");
        return 1;
    }

    // Check if hooks directory exists if provided
    if (!options.hooks_dir.empty() && !std::filesystem::exists(options.hooks_dir)) {
        dpm_log(LOG_ERROR, ("Hooks directory does not exist: " + options.hooks_dir).c_str());
        return 1;
    }

    return 0;
}