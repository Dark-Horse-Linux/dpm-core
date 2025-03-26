#include "commands.hpp"

int cmd_metadata(int argc, char** argv) {
    // Parse command line options
    bool force = false;
    bool refresh = false;
    bool verbose = false;
    bool show_help = false;
    std::string stage_dir = "";
    std::string package_name = "";
    std::string package_version = "";
    std::string architecture = "";

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "-r" || arg == "--refresh") {
            refresh = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        } else if ((arg == "-s" || arg == "--stage") && i + 1 < argc) {
            stage_dir = argv[i + 1];
            i++; // Skip the next argument
        } else if ((arg == "-n" || arg == "--name") && i + 1 < argc) {
            package_name = argv[i + 1];
            i++; // Skip the next argument
        } else if ((arg == "-V" || arg == "--version") && i + 1 < argc) {
            package_version = argv[i + 1];
            i++; // Skip the next argument
        } else if ((arg == "-a" || arg == "--architecture") && i + 1 < argc) {
            architecture = argv[i + 1];
            i++; // Skip the next argument
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_metadata_help(argc, argv);
    }

    // Validate that stage directory is provided
    if (stage_dir.empty()) {
        dpm_log(LOG_ERROR, "Package stage directory is required (--stage/-s)");
        return cmd_metadata_help(argc, argv);
    }

    // Expand path if needed
    stage_dir = expand_path(stage_dir);

    // Check if stage directory exists
    if (!std::filesystem::exists(stage_dir)) {
        dpm_log(LOG_ERROR, ("Stage directory does not exist: " + stage_dir).c_str());
        return 1;
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Call the appropriate function based on the refresh flag
    if (refresh) {
        // For refresh mode, we only need the stage directory
        bool success = metadata_refresh_dynamic_files(stage_dir);
        if (!success) {
            dpm_log(LOG_ERROR, "Failed to refresh metadata files.");
            return 1;
        }
        dpm_log(LOG_INFO, "Metadata files refreshed successfully.");
        return 0;
    } else {
        // For generate mode, we need additional parameters
        if (package_name.empty()) {
            dpm_log(LOG_ERROR, "Package name is required for metadata generation (--name/-n)");
            return cmd_metadata_help(argc, argv);
        }

        if (package_version.empty()) {
            dpm_log(LOG_ERROR, "Package version is required for metadata generation (--version/-V)");
            return cmd_metadata_help(argc, argv);
        }

        if (architecture.empty()) {
            dpm_log(LOG_ERROR, "Package architecture is required for metadata generation (--architecture/-a)");
            return cmd_metadata_help(argc, argv);
        }

        bool success = metadata_generate_new(
            std::filesystem::path(stage_dir),
            package_name,
            package_version,
            architecture
        );

        if (!success) {
            dpm_log(LOG_ERROR, "Failed to generate metadata files.");
            return 1;
        }

        dpm_log(LOG_INFO, "Metadata files generated successfully.");
        return 0;
    }
}

int cmd_stage(int argc, char** argv) {
    // Announce that the stage step is being executed (debug level)
    dpm_log(LOG_DEBUG, "Executing stage command");

    // create a container for commandline options
    BuildOptions options;

    // Parse command-line options
    int parse_result = parse_create_options(argc, argv, options);
    if (parse_result != 0) {
        return parse_result;
    }

    // If help was requested, show it and return
    if (options.show_help) {
        return cmd_stage_help(argc, argv);
    }

    // Set logging level to DEBUG when verbose is enabled
    if (options.verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // If OS is not supplied, try to get it from config
    if (options.os.empty()) {
        dpm_log( LOG_DEBUG, "Target OS not provided as a commandline argument.");
        const char* config_os = dpm_get_config("build", "os");
        if (config_os != nullptr) {
            options.os = config_os;
            dpm_log(LOG_DEBUG, ("Using build.os from config: " + options.os).c_str());
        } else {
            dpm_log(LOG_ERROR, "Target OS not specified and not found as build.os in configuration.");
            dpm_log(LOG_ERROR, "Please specify OS with --os or set a default at build.os in '/etc/dpm/conf.d/'.");
            return 1;
        }
    }

    // Validate options
    int validate_result = validate_build_options(options);
    if (validate_result != 0) {
        cmd_stage_help(argc, argv);
        return validate_result;
    }

    // Log detailed options (only visible in verbose mode)
    dpm_log(LOG_DEBUG, "Staging DPM package with the following options:");
    dpm_log(LOG_DEBUG, ("  Output directory: " + options.output_dir).c_str());
    dpm_log(LOG_DEBUG, ("  Contents directory: " + options.contents_dir).c_str());
    dpm_log(LOG_DEBUG, ("  Package name: " + options.package_name).c_str());
    dpm_log(LOG_DEBUG, ("  Package version: " + options.package_version).c_str());
    dpm_log(LOG_DEBUG, ("  Architecture: " + options.architecture).c_str());
    dpm_log(LOG_DEBUG, ("  OS: " + options.os).c_str());

    if (!options.hooks_dir.empty()) {
        dpm_log(LOG_DEBUG, ("  Hooks directory: " + options.hooks_dir).c_str());
    } else {
        dpm_log(LOG_DEBUG, "  Hooks directory: N/A");
    }

    if (options.force) {
        dpm_log(LOG_DEBUG, "  Force: Yes");
    }

    // Call the build_package_stage function with individual parameters
    return build_package_stage(
        options.output_dir,
        options.contents_dir,
        options.hooks_dir,
        options.package_name,
        options.package_version,
        options.architecture,
        options.os,
        options.force
    );
}

int cmd_sign(int argc, char** argv) {
    // Parse command line options
    std::string key_id = "";
    std::string stage_dir = "";
    std::string package_path = "";
    bool force = false;
    bool verbose = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-k" || arg == "--key-id") {
            if (i + 1 < argc) {
                key_id = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-s" || arg == "--stage") {
            if (i + 1 < argc) {
                stage_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-p" || arg == "--package") {
            if (i + 1 < argc) {
                package_path = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_sign_help(argc, argv);
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Validate that key ID is provided
    if (key_id.empty()) {
        dpm_log(LOG_ERROR, "GPG key ID is required (--key-id/-k)");
        return cmd_sign_help(argc, argv);
    }

    // Validate that either stage or package is provided, but not both
    if (stage_dir.empty() && package_path.empty()) {
        dpm_log(LOG_ERROR, "Either a package stage directory (--stage/-s) or a package file (--package/-p) must be specified");
        return cmd_sign_help(argc, argv);
    }

    if (!stage_dir.empty() && !package_path.empty()) {
        dpm_log(LOG_ERROR, "Cannot specify both package stage directory (--stage/-s) and package file (--package/-p)");
        return cmd_sign_help(argc, argv);
    }

    // Expand paths if needed
    if (!stage_dir.empty()) {
        stage_dir = expand_path(stage_dir);
        // Check if stage directory exists
        if (!std::filesystem::exists(stage_dir)) {
            dpm_log(LOG_ERROR, ("Stage directory does not exist: " + stage_dir).c_str());
            return 1;
        }

        // Sign the stage directory
        return sign_stage_directory(stage_dir, key_id, force);
    } else {
        package_path = expand_path(package_path);
        // Check if package file exists
        if (!std::filesystem::exists(package_path)) {
            dpm_log(LOG_ERROR, ("Package file does not exist: " + package_path).c_str());
            return 1;
        }

        // Sign the package file
        return sign_package_file(package_path, key_id, force);
    }
}

int cmd_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "DPM Build Module - Creates DPM packages.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Available commands:");
    dpm_con(LOG_INFO, "  stage      - Stage a new DPM package directory");
    dpm_con(LOG_INFO, "  metadata   - Generate or refresh package metadata");
    dpm_con(LOG_INFO, "  sign       - Sign a package or package stage directory");
    dpm_con(LOG_INFO, "  seal       - Seal a package stage directory into final format");
    dpm_con(LOG_INFO, "  unseal     - Unseal a package back to stage format");
    dpm_con(LOG_INFO, "  help       - Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Usage: dpm build <command>");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "For command-specific help, use: dpm build <command> --help");

    return 0;
}

int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_con(LOG_WARN, msg.c_str());
    dpm_con(LOG_WARN, "Run 'dpm build help' for a list of available commands");
    return 1;
}


int cmd_metadata_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm build metadata [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -s, --stage DIR           Package stage directory path (required)");
    dpm_con(LOG_INFO, "  -r, --refresh             Refresh existing metadata (use for updating)");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "For new metadata generation (when not using --refresh):");
    dpm_con(LOG_INFO, "  -n, --name NAME           Package name (required for new generation)");
    dpm_con(LOG_INFO, "  -V, --version VERSION     Package version (required for new generation)");
    dpm_con(LOG_INFO, "  -a, --architecture ARCH   Package architecture (required for new generation)");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Additional options:");
    dpm_con(LOG_INFO, "  -f, --force               Force operation even if warnings occur");
    dpm_con(LOG_INFO, "  -v, --verbose             Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help                Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  # Refresh metadata in an existing package stage:");
    dpm_con(LOG_INFO, "  dpm build metadata --stage=./my-package-1.0.x86_64 --refresh");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "  # Generate new metadata for a package stage:");
    dpm_con(LOG_INFO, "  dpm build metadata --stage=./my-package-1.0.x86_64 --name=my-package --version=1.0 --architecture=x86_64");
    return 0;
}

int cmd_stage_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm build stage [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -o, --output DIR           Directory to save the staged package (required)");
    dpm_con(LOG_INFO, "  -c, --contents DIR         Directory with package contents (required)");
    dpm_con(LOG_INFO, "  -H, --hooks DIR            Directory with package hooks (optional)");
    dpm_con(LOG_INFO, "  -n, --name NAME            Package name (required)");
    dpm_con(LOG_INFO, "  -V, --version VERSION      Package version (required)");
    dpm_con(LOG_INFO, "  -a, --architecture ARCH    Package architecture (required, e.g., x86_64)");
    dpm_con(LOG_INFO, "  -O, --os OS                Package OS (optional, e.g., dhl2)");
    dpm_con(LOG_INFO, "  -f, --force                Force package staging even if warnings occur");
    dpm_con(LOG_INFO, "  -v, --verbose              Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help                 Display this help message");
    return 0;
}

int cmd_sign_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm build sign [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Sign a DPM package or package stage directory using GPG.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -k, --key-id ID          GPG key ID or email to use for signing (required)");
    dpm_con(LOG_INFO, "  -s, --stage DIR          Package stage directory to sign");
    dpm_con(LOG_INFO, "  -p, --package FILE       Package file to sign");
    dpm_con(LOG_INFO, "  -f, --force              Force signing even if warnings occur");
    dpm_con(LOG_INFO, "  -v, --verbose            Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help               Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Either --stage or --package must be specified, but not both.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  dpm build sign --key-id=\"user@example.com\" --stage=./my-package-1.0.x86_64");
    dpm_con(LOG_INFO, "  dpm build sign --key-id=\"AB123CD456\" --package=./my-package-1.0.x86_64.dpm");
    return 0;
}

int cmd_unseal(int argc, char** argv) {
    // Parse command line options
    std::string input_path = "";
    std::string output_dir = "";
    bool components_mode = false;
    bool force = false;
    bool verbose = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) {
                input_path = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                output_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-c" || arg == "--components") {
            components_mode = true;
        } else if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_unseal_help(argc, argv);
    }

    // Validate that input path is provided
    if (input_path.empty()) {
        dpm_con(LOG_ERROR, "Input path is required (--input/-i)");
        return cmd_unseal_help(argc, argv);
    }

    // Check for invalid option combinations
    if (components_mode && !output_dir.empty()) {
        dpm_con(LOG_ERROR, "Output directory (-o/--output) cannot be specified in components mode (-c/--components)");
        return cmd_unseal_help(argc, argv);
    }

    // Expand path if needed
    input_path = expand_path(input_path);

    // Check if input path exists
    if (!std::filesystem::exists(input_path)) {
        dpm_con(LOG_ERROR, ("Input path does not exist: " + input_path).c_str());
        return 1;
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Determine which operation to perform based on components_mode flag
    if (components_mode) {
        // We're unsealing components of a stage directory
        if (!std::filesystem::is_directory(input_path)) {
            dpm_con(LOG_ERROR, ("Input path must be a directory in components mode: " + input_path).c_str());
            return 1;
        }

        // Call unseal_stage_components with just the input path
        return unseal_stage_components(input_path);
    } else {
        // We're unsealing a package file
        if (std::filesystem::is_directory(input_path)) {
            dpm_con(LOG_ERROR, ("Input path must be a file when not in components mode: " + input_path).c_str());
            return 1;
        }

        // Call unseal_package
        return unseal_package(input_path, output_dir, force);
    }
}

int cmd_unseal_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm build unseal [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Unseals a DPM package file or package stage components.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -i, --input PATH       Path to package file or stage directory (required)");
    dpm_con(LOG_INFO, "  -o, --output DIR       Directory to extract package to (optional, package mode only)");
    dpm_con(LOG_INFO, "  -c, --components       Component mode: unseal components in a stage directory");
    dpm_con(LOG_INFO, "                         Without this flag, input is treated as a package file");
    dpm_con(LOG_INFO, "  -f, --force            Force unsealing even if warnings occur or directory exists");
    dpm_con(LOG_INFO, "  -v, --verbose          Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help             Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  # Unseal a package file to a directory:");
    dpm_con(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64.dpm");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "  # Unseal a package file to a specific directory:");
    dpm_con(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64.dpm --output=./extract");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "  # Unseal components in a stage directory:");
    dpm_con(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64 --components");
    return 0;
}

int cmd_seal(int argc, char** argv) {
    // Parse command line options
    std::string stage_dir = "";
    std::string output_dir = "";
    bool force = false;
    bool verbose = false;
    bool finalize = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-s" || arg == "--stage") {
            if (i + 1 < argc) {
                stage_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                output_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "-z" || arg == "--finalize") {
            finalize = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_seal_help(argc, argv);
    }

    // Validate that stage directory is provided
    if (stage_dir.empty()) {
        dpm_con(LOG_ERROR, "Stage directory is required (--stage/-s)");
        return cmd_seal_help(argc, argv);
    }

    // Expand path if needed
    stage_dir = expand_path(stage_dir);

    // Check if stage directory exists
    if (!std::filesystem::exists(stage_dir)) {
        dpm_con(LOG_ERROR, ("Stage directory does not exist: " + stage_dir).c_str());
        return 1;
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Call the appropriate sealing function based on the finalize flag
    if (finalize) {
        return seal_final_package(stage_dir, output_dir, force);
    } else {
        return seal_stage_components(stage_dir, force);
    }
}

int cmd_seal_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm build seal [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Seals a package stage directory by replacing contents, metadata,");
    dpm_con(LOG_INFO, "hooks, and signatures directories with gzipped tarballs.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -s, --stage DIR         Package stage directory to seal (required)");
    dpm_con(LOG_INFO, "  -o, --output DIR        Output directory for the finalized package (optional)");
    dpm_con(LOG_INFO, "  -f, --force             Force sealing even if warnings occur");
    dpm_con(LOG_INFO, "  -z, --finalize          Also compress the entire stage as a final package");
    dpm_con(LOG_INFO, "  -v, --verbose           Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help              Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64");
    dpm_con(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64 --finalize");
    dpm_con(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64 --finalize --output=/tmp");
    return 0;
}
