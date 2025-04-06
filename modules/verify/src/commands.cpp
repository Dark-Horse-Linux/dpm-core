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

/**
 * @brief Verifies checksums of a package file in memory
 *
 * Loads the components of a package file into memory and verifies their checksums
 * without extracting them to disk.
 *
 * @param package_path Path to the package file
 * @return 0 on success, non-zero on failure
 */
int verify_checksums_package_memory(const std::string& package_path) {
    // Check if the package file exists
    if (!std::filesystem::exists(package_path)) {
        dpm_log(LOG_ERROR, ("Package file not found: " + package_path).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, ("Verifying checksums for package in memory: " + package_path).c_str());

    // Load the build module
    void* build_module = nullptr;
    int result = check_and_load_build_module(build_module);
    if (result != 0 || build_module == nullptr) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return 1;
    }

    // Extract package components into memory
    unsigned char* metadata_data = nullptr;
    size_t metadata_data_size = 0;
    unsigned char* contents_data = nullptr;
    size_t contents_data_size = 0;
    unsigned char* hooks_data = nullptr;
    size_t hooks_data_size = 0;

    // Load metadata component
    dpm_log(LOG_INFO, "Loading metadata component...");
    result = get_component_from_package(package_path, "metadata", &metadata_data, &metadata_data_size);
    if (result != 0 || !metadata_data || metadata_data_size == 0) {
        dpm_log(LOG_ERROR, "Failed to load metadata component");
        dpm_unload_module(build_module);
        return 1;
    }

    // Load contents component
    dpm_log(LOG_INFO, "Loading contents component...");
    result = get_component_from_package(package_path, "contents", &contents_data, &contents_data_size);
    if (result != 0 || !contents_data || contents_data_size == 0) {
        dpm_log(LOG_ERROR, "Failed to load contents component");
        free(metadata_data);
        dpm_unload_module(build_module);
        return 1;
    }

    // Load hooks component
    dpm_log(LOG_INFO, "Loading hooks component...");
    result = get_component_from_package(package_path, "hooks", &hooks_data, &hooks_data_size);
    if (result != 0 || !hooks_data || hooks_data_size == 0) {
        dpm_log(LOG_ERROR, "Failed to load hooks component");
        free(metadata_data);
        free(contents_data);
        dpm_unload_module(build_module);
        return 1;
    }

    // Verify package digest
    dpm_log(LOG_INFO, "Verifying package digest...");
    result = checksum_verify_package_digest_memory(
        metadata_data,
        metadata_data_size,
        build_module
    );
    if (result != 0) {
        dpm_log(LOG_ERROR, "Package digest verification failed");
        free(metadata_data);
        free(contents_data);
        free(hooks_data);
        dpm_unload_module(build_module);
        return 1;
    }

    // Verify contents manifest digest
    dpm_log(LOG_INFO, "Verifying contents manifest digest...");
    result = checksum_verify_contents_digest_memory(
        contents_data,
        contents_data_size,
        metadata_data,
        metadata_data_size,
        build_module
    );
    if (result != 0) {
        dpm_log(LOG_ERROR, "Contents manifest verification failed");
        free(metadata_data);
        free(contents_data);
        free(hooks_data);
        dpm_unload_module(build_module);
        return 1;
    }

    // Verify hooks digest
    dpm_log(LOG_INFO, "Verifying hooks digest...");
    result = checksum_verify_hooks_digest_memory(
        hooks_data,
        hooks_data_size,
        metadata_data,
        metadata_data_size,
        build_module
    );
    if (result != 0) {
        dpm_log(LOG_ERROR, "Hooks digest verification failed");
        free(metadata_data);
        free(contents_data);
        free(hooks_data);
        dpm_unload_module(build_module);
        return 1;
    }

    // Clean up
    free(metadata_data);
    free(contents_data);
    free(hooks_data);
    dpm_unload_module(build_module);

    dpm_log(LOG_INFO, "All in-memory checksums verified successfully");
    return 0;
}