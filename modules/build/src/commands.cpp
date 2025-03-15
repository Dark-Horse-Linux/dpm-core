#include "commands.hpp"


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