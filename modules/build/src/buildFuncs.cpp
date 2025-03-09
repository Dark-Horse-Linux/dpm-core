/**
 * @file buildFuncs.cpp
 * @brief Implementation of the build module support functions
 *
 * Implements functions for the build module that create DPM packages
 * according to the specification.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "buildFuncs.hpp"
#include <iostream>
#include <filesystem>

/**
 * @brief Handler for the help command
 */
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

/**
 * @brief Handler for unknown commands
 */
int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_log(LOG_WARN, msg.c_str());
    dpm_log(LOG_WARN, "Run 'dpm build help' for a list of available commands");
    return 1;
}

/**
 * @brief Parses a command string to Command enum
 */
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

/**
 * @brief Handler for the create command
 */
int cmd_create(int argc, char** argv) {
    BuildOptions options;

    // Parse command-line options
    int parse_result = parse_create_options(argc, argv, options);
    if (parse_result != 0) {
        return parse_result;
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

/**
 * @brief Parses command-line arguments for the create command
 */
int parse_create_options(int argc, char** argv, BuildOptions& options) {
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
        {0, 0, 0, 0}
    };

    // Skip program name and module name
    int adjusted_argc = argc;
    char** adjusted_argv = argv;

    // Reset getopt
    optind = 0;
    opterr = 1;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(adjusted_argc, adjusted_argv, "o:c:m:H:n:s:fvh", long_options, &option_index)) != -1) {
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
                cmd_help(0, nullptr);
                return 1;
            case '?':
                // Error message is printed by getopt
                dpm_log(LOG_ERROR, "Run 'dpm build create --help' for usage information");
                return 1;
            default:
                dpm_log(LOG_ERROR, "Unknown option");
                return 1;
        }
    }

    return 0;
}

/**
 * @brief Validates the build options
 */
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
