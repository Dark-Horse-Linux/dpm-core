#include "commands.hpp"

int cmd_create(int argc, char** argv) {
    // create a container for commandline options
    BuildOptions options;

    // Parse command-line options
    int parse_result = parse_create_options(argc, argv, options);
    if (parse_result != 0) {
        return parse_result;
    }

    // If help was requested, show it and return
    if (options.show_help) {
        return cmd_help(argc, argv);
    }

    // Validate options
    int validate_result = validate_build_options(options);
    if (validate_result != 0) {
        return validate_result;
    }

    // Log the operation
    if (options.verbose) {
        dpm_log(LOG_INFO, "Creating DPM package with the following options:");
        dpm_log(LOG_INFO, ("  Output directory: " + options.output_dir).c_str());
        dpm_log(LOG_INFO, ("  Contents directory: " + options.contents_dir).c_str());
        dpm_log(LOG_INFO, ("  Metadata directory: " + options.metadata_dir).c_str());

        if (!options.hooks_dir.empty()) {
            dpm_log(LOG_INFO, ("  Hooks directory: " + options.hooks_dir).c_str());
        }

        if (!options.package_name.empty()) {
            dpm_log(LOG_INFO, ("  Package name: " + options.package_name).c_str());
        }

        if (!options.signature_key.empty()) {
            dpm_log(LOG_INFO, ("  Signature key: " + options.signature_key).c_str());
        }

        if (options.force) {
            dpm_log(LOG_INFO, "  Force: Yes");
        }
    }

    // For now, just log that we would create the package
    dpm_log(LOG_INFO, "Package creation functionality not yet implemented");
    dpm_log(LOG_INFO, "Would create package using the provided options");

    return 0;
}

int cmd_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "DPM Build Module - Creates DPM packages according to specification");
    dpm_log(LOG_INFO, "Available commands:");
    dpm_log(LOG_INFO, "  create     - Create a new DPM package");
    dpm_log(LOG_INFO, "  help       - Display this help message");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Usage: dpm build create [options]");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -o, --output-dir DIR    Directory to save the built package (default: current directory)");
    dpm_log(LOG_INFO, "  -c, --contents DIR      Directory with package contents (required)");
    dpm_log(LOG_INFO, "  -m, --metadata DIR      Directory with package metadata (required)");
    dpm_log(LOG_INFO, "  -H, --hooks DIR         Directory with package hooks (optional)");
    dpm_log(LOG_INFO, "  -n, --name NAME         Package name (required if not in metadata)");
    dpm_log(LOG_INFO, "  -s, --sign KEY          Path to GPG key for signing the package (optional)");
    dpm_log(LOG_INFO, "  -f, --force             Force package creation even if warnings occur");
    dpm_log(LOG_INFO, "  -v, --verbose           Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help              Display this help message");
    return 0;
}

int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_log(LOG_WARN, msg.c_str());
    dpm_log(LOG_WARN, "Run 'dpm build help' for a list of available commands");
    return 1;
}



