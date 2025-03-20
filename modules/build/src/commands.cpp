#include "commands.hpp"

static int refresh_package_manifest(const std::string& stage_dir, bool force) {
    dpm_log(LOG_INFO, ("Refreshing package manifest for: " + stage_dir).c_str());
    return 0;
}

static int generate_package_manifest(
    const std::string& package_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture,
    bool force
) {
    dpm_log(LOG_INFO, ("Generating package manifest for: " + package_dir).c_str());

    // Generate the metadata files using the provided information
    if (!metadata_generate_new(std::filesystem::path(package_dir), package_name, package_version, architecture)) {
        dpm_log(LOG_ERROR, "Failed to generate metadata.");
        return 1;
    }

    dpm_log(LOG_INFO, "Package content manifest generated successfully.");
    return 0;
}

int cmd_manifest(int argc, char** argv) {
    // Parse command line options
    bool force = false;
    bool replace = false;
    bool verbose = false;
    bool show_help = false;
    std::string package_dir = "";
    std::string package_name = "";
    std::string package_version = "";
    std::string architecture = "";

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "-r" || arg == "--replace") {
            replace = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        } else if ((arg == "-p" || arg == "--package-dir") && i + 1 < argc) {
            package_dir = argv[i + 1];
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
        return cmd_manifest_help(argc, argv);
    }

    // Validate that package directory is provided
    if (package_dir.empty()) {
        dpm_log(LOG_ERROR, "Package directory is required (--package-dir/-p)");
        return cmd_manifest_help(argc, argv);
    }

    // Expand path if needed
    package_dir = expand_path(package_dir);

    // Check if package directory exists
    if (!std::filesystem::exists(package_dir)) {
        dpm_log(LOG_ERROR, ("Package directory does not exist: " + package_dir).c_str());
        return 1;
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Log the operation being performed
    if (replace) {
        // When replacing a manifest, we need name, version, and architecture
        if (package_name.empty() || package_version.empty() || architecture.empty()) {
            dpm_log(LOG_ERROR, "Package name, version, and architecture are required for replacing a manifest");
            return cmd_manifest_help(argc, argv);
        }

        return generate_package_manifest(package_dir, package_name, package_version, architecture, force);
    } else {
        return refresh_package_manifest(package_dir, force);
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

int cmd_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "DPM Build Module - Creates DPM packages according to specification.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Available commands:");
    dpm_log(LOG_INFO, "  stage      - Stage a new DPM package directory");
    dpm_log(LOG_INFO, "  manifest   - Generate or refresh package manifest");
    dpm_log(LOG_INFO, "  help       - Display this help message");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Usage: dpm build <command>");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "For command-specific help, use: dpm build <command> --help");

    return 0;
}

int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_log(LOG_WARN, msg.c_str());
    dpm_log(LOG_WARN, "Run 'dpm build help' for a list of available commands");
    return 1;
}


int cmd_manifest_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "Usage: dpm build manifest [options]");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -p, --package-dir DIR     Package directory path (required)");
    dpm_log(LOG_INFO, "  -n, --name NAME           Package name (required when replacing)");
    dpm_log(LOG_INFO, "  -V, --version VERSION     Package version (required when replacing)");
    dpm_log(LOG_INFO, "  -a, --architecture ARCH   Package architecture (required when replacing)");
    dpm_log(LOG_INFO, "  -r, --replace             Replace manifest with new one (default: refresh existing)");
    dpm_log(LOG_INFO, "  -f, --force               Force manifest operation even if warnings occur");
    dpm_log(LOG_INFO, "  -v, --verbose             Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help                Display this help message");
    dpm_log(LOG_INFO, "");
    return 0;
}


int cmd_stage_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "Usage: dpm build stage [options]");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -o, --output DIR           Directory to save the staged package (required)");
    dpm_log(LOG_INFO, "  -c, --contents DIR         Directory with package contents (required)");
    dpm_log(LOG_INFO, "  -H, --hooks DIR            Directory with package hooks (optional)");
    dpm_log(LOG_INFO, "  -n, --name NAME            Package name (required)");
    dpm_log(LOG_INFO, "  -V, --version VERSION      Package version (required)");
    dpm_log(LOG_INFO, "  -a, --architecture ARCH    Package architecture (required, e.g., x86_64)");
    dpm_log(LOG_INFO, "  -O, --os OS                Package OS (optional, e.g., dhl2)");
    dpm_log(LOG_INFO, "  -f, --force                Force package staging even if warnings occur");
    dpm_log(LOG_INFO, "  -v, --verbose              Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help                 Display this help message");
    return 0;
}
