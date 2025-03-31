/**
 * @file commands.cpp
 * @brief Implementation of command handlers for the verify module
 *
 * Implements the command handlers for verifying package checksums and signatures.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "commands.hpp"

int check_and_load_build_module(void*& module_handle) {
    // Check if build module exists
    if (!dpm_module_exists("build")) {
        dpm_log(LOG_ERROR, "Build module not found");
        return 1;
    }

    // Load the build module
    int result = dpm_load_module("build", &module_handle);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return result;
    }

    return 0;
}

int cmd_checksum_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm verify checksum [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Verifies the checksums of packages or package stage directories.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -p, --package PATH     Path to a package file (.dpm)");
    dpm_con(LOG_INFO, "  -s, --stage DIR        Path to a package stage directory");
    dpm_con(LOG_INFO, "  -v, --verbose          Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help             Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Note: --package and --stage are mutually exclusive options.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  dpm verify checksum --package=mypackage-1.0.x86_64.dpm");
    dpm_con(LOG_INFO, "  dpm verify checksum --stage=./mypackage-1.0.x86_64");
    return 0;
}

int cmd_checksum(int argc, char** argv) {
    // Parse command line arguments
    std::string package_path = "";
    std::string stage_dir = "";
    bool verbose = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-p" || arg == "--package") {
            if (i + 1 < argc) {
                package_path = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-s" || arg == "--stage") {
            if (i + 1 < argc) {
                stage_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_checksum_help(argc, argv);
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Validate that either package_path or stage_dir is provided, but not both
    if (package_path.empty() && stage_dir.empty()) {
        dpm_con(LOG_ERROR, "Either --package or --stage must be specified");
        return cmd_checksum_help(argc, argv);
    }

    if (!package_path.empty() && !stage_dir.empty()) {
        dpm_con(LOG_ERROR, "Cannot specify both --package and --stage");
        return cmd_checksum_help(argc, argv);
    }

    // Call the appropriate verification function
    if (!package_path.empty()) {
        return verify_checksums_package(package_path);
    } else {
        return verify_checksums_stage(stage_dir);
    }
}

int cmd_signature_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm verify signature [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Verifies the signatures of packages or package stage directories.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -p, --package PATH     Path to a package file (.dpm)");
    dpm_con(LOG_INFO, "  -s, --stage DIR        Path to a package stage directory");
    dpm_con(LOG_INFO, "  -v, --verbose          Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help             Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Note: --package and --stage are mutually exclusive options.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Examples:");
    dpm_con(LOG_INFO, "  dpm verify signature --package=mypackage-1.0.x86_64.dpm");
    dpm_con(LOG_INFO, "  dpm verify signature --stage=./mypackage-1.0.x86_64");
    return 0;
}

int cmd_signature(int argc, char** argv) {
    // Parse command line arguments
    std::string package_path = "";
    std::string stage_dir = "";
    bool verbose = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-p" || arg == "--package") {
            if (i + 1 < argc) {
                package_path = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-s" || arg == "--stage") {
            if (i + 1 < argc) {
                stage_dir = argv[i + 1];
                i++; // Skip the next argument
            }
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_signature_help(argc, argv);
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    // Validate that either package_path or stage_dir is provided, but not both
    if (package_path.empty() && stage_dir.empty()) {
        dpm_con(LOG_ERROR, "Either --package or --stage must be specified");
        return cmd_signature_help(argc, argv);
    }

    if (!package_path.empty() && !stage_dir.empty()) {
        dpm_con(LOG_ERROR, "Cannot specify both --package and --stage");
        return cmd_signature_help(argc, argv);
    }

    // Call the appropriate verification function
    if (!package_path.empty()) {
        return verify_signature_package(package_path);
    } else {
        return verify_signature_stage(stage_dir);
    }
}

int cmd_check_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "Usage: dpm verify check [options]");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Checks build module integration.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Options:");
    dpm_con(LOG_INFO, "  -v, --verbose          Enable verbose output");
    dpm_con(LOG_INFO, "  -h, --help             Display this help message");
    return 0;
}

int cmd_help(int argc, char** argv) {
    dpm_con(LOG_INFO, "DPM Verify Module - Verifies the integrity and signatures of package files and stage directories.");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Available commands:");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "  checksum   - Verify checksums of package files or stage directories");
    dpm_con(LOG_INFO, "  signature  - Verify signatures of package files or stage directories");
    dpm_con(LOG_INFO, "  check      - Check build module integration");
    dpm_con(LOG_INFO, "  help       - Display this help message");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "Usage: dpm verify <command>");
    dpm_con(LOG_INFO, "");
    dpm_con(LOG_INFO, "For command-specific help, use: dpm verify <command> --help");

    return 0;
}

int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_con(LOG_WARN, msg.c_str());
    dpm_con(LOG_WARN, "Run 'dpm verify help' for a list of available commands");
    return 1;
}

int cmd_check(int argc, char** argv) {
    // Parse command line arguments
    bool verbose = false;
    bool show_help = false;

    // Process command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help" || arg == "help") {
            show_help = true;
        }
    }

    // If help was requested, show it and return
    if (show_help) {
        return cmd_check_help(argc, argv);
    }

    // Set verbose logging if requested
    if (verbose) {
        dpm_set_logging_level(LOG_DEBUG);
    }

    dpm_log(LOG_INFO, "Checking build module integration...");

    void* module_handle = nullptr;
    int result = check_and_load_build_module(module_handle);

    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to load build module.");
        return 1;
    }

    dpm_log(LOG_INFO, "Successfully loaded build module.");

    // Check if the dpm_module_execute symbol exists
    if (!dpm_symbol_exists(module_handle, "dpm_module_execute")) {
        dpm_log(LOG_ERROR, "Symbol 'dpm_module_execute' not found in build module.");
        dpm_unload_module(module_handle);
        return 1;
    }

    dpm_log(LOG_INFO, "Symbol 'dpm_module_execute' found in build module.");

    // Clean up
    if (module_handle) {
        dpm_unload_module(module_handle);
    }

    return 0;
}