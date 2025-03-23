#include "commands.hpp"

/**
 * @brief Refreshes the contents manifest file by updating checksums
 *
 * Iterates through the existing CONTENTS_MANIFEST_DIGEST file, rereads each file,
 * recalculates its checksum, and updates the file with new checksums while
 * preserving all other fields.
 *
 * @param stage_dir Directory path of the package stage
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
static int refresh_contents_manifest(const std::string& stage_dir, bool force) {
    dpm_log(LOG_INFO, ("Refreshing package manifest for: " + stage_dir).c_str());

    std::filesystem::path package_dir = std::filesystem::path(stage_dir);
    std::filesystem::path contents_dir = package_dir / "contents";
    std::filesystem::path manifest_path = package_dir / "metadata" / "CONTENTS_MANIFEST_DIGEST";

    // Check if contents directory exists
    if (!std::filesystem::exists(contents_dir)) {
        dpm_log(LOG_ERROR, ("Contents directory does not exist: " + contents_dir.string()).c_str());
        return 1;
    }

    // Map to track all files in the contents directory
    std::map<std::filesystem::path, bool> all_content_files;

    // Populate map with all files in contents directory
    for (const auto& entry : std::filesystem::recursive_directory_iterator(contents_dir)) {
        if (!std::filesystem::is_directory(entry)) {
            // Store path relative to contents directory
            std::filesystem::path relative_path = std::filesystem::relative(entry.path(), contents_dir);
            all_content_files[relative_path] = false; // Not processed yet
        }
    }

    // Check if manifest file exists
    bool manifest_exists = std::filesystem::exists(manifest_path);

    // Create a temporary file for the updated manifest
    std::filesystem::path temp_manifest_path = manifest_path.string() + ".tmp";
    std::ofstream temp_manifest_file(temp_manifest_path);
    if (!temp_manifest_file.is_open()) {
        dpm_log(LOG_ERROR, ("Failed to create temporary manifest file: " + temp_manifest_path.string()).c_str());
        return 1;
    }

    // Log which hash algorithm is being used
    std::string hash_algorithm = get_configured_hash_algorithm();
    dpm_log(LOG_INFO, ("Refreshing contents manifest using " + hash_algorithm + " checksums...").c_str());

    int updated_files = 0;
    int new_files = 0;

    // First process existing manifest file if it exists
    if (manifest_exists) {
        std::ifstream manifest_file(manifest_path);
        if (!manifest_file.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open manifest file for reading: " + manifest_path.string()).c_str());
            temp_manifest_file.close();
            std::filesystem::remove(temp_manifest_path);
            return 1;
        }

        std::string line;
        int line_number = 0;

        // Process each line in the manifest
        while (std::getline(manifest_file, line)) {
            line_number++;

            // Skip empty lines
            if (line.empty()) {
                temp_manifest_file << line << std::endl;
                continue;
            }

            // Parse the line into its components
            std::istringstream iss(line);
            char control_designation;
            std::string checksum, permissions, ownership, file_path;

            // Extract components (C checksum permissions owner:group /path/to/file)
            iss >> control_designation >> checksum >> permissions >> ownership;

            // The file path might contain spaces, so we need to get the rest of the line
            std::getline(iss >> std::ws, file_path);

            // Skip if we couldn't parse the line correctly
            if (file_path.empty()) {
                dpm_log(LOG_WARN, ("Skipping malformed line " + std::to_string(line_number) + ": " + line).c_str());
                temp_manifest_file << line << std::endl;
                continue;
            }

            // Remove leading slash from file_path if present
            if (file_path[0] == '/') {
                file_path = file_path.substr(1);
            }

            // Mark this file as processed
            std::filesystem::path relative_path(file_path);
            if (all_content_files.find(relative_path) != all_content_files.end()) {
                all_content_files[relative_path] = true; // Mark as processed
            }

            // Construct the full path to the file in the contents directory
            std::filesystem::path full_file_path = contents_dir / file_path;

            // Check if the file exists
            if (!std::filesystem::exists(full_file_path)) {
                dpm_log(LOG_WARN, ("File not found in contents directory: " + full_file_path.string()).c_str());
                // Keep the original line
                temp_manifest_file << control_designation << " "
                                  << checksum << " "
                                  << permissions << " "
                                  << ownership << " "
                                  << "/" << file_path << std::endl;
                continue;
            }

            // Calculate new checksum
            std::string new_checksum = generate_file_checksum(full_file_path);
            if (new_checksum.empty()) {
                dpm_log(LOG_ERROR, ("Failed to generate checksum for: " + full_file_path.string()).c_str());
                manifest_file.close();
                temp_manifest_file.close();
                std::filesystem::remove(temp_manifest_path);
                return 1;
            }

            // Write updated line to the temporary file
            temp_manifest_file << control_designation << " "
                              << new_checksum << " "
                              << permissions << " "
                              << ownership << " "
                              << "/" << file_path << std::endl;

            // Count updated files (only if checksum actually changed)
            if (new_checksum != checksum) {
                updated_files++;
            }
        }

        manifest_file.close();
    }

    // Now process any new files not in the manifest
    for (const auto& [file_path, processed] : all_content_files) {
        // Skip if already processed from manifest
        if (processed) {
            continue;
        }

        // This is a new file
        std::filesystem::path full_file_path = contents_dir / file_path;

        // Get file stats for permissions
        struct stat file_stat;
        if (stat(full_file_path.c_str(), &file_stat) != 0) {
            dpm_log(LOG_ERROR, ("Failed to get file stats for: " + full_file_path.string()).c_str());
            continue;
        }

        // Format permissions as octal
        char perms[5];
        snprintf(perms, sizeof(perms), "%04o", file_stat.st_mode & 07777);

        // Get owner and group information
        struct passwd* pw = getpwuid(file_stat.st_uid);
        struct group* gr = getgrgid(file_stat.st_gid);

        std::string owner;
        if (pw) {
            owner = pw->pw_name;
        } else {
            owner = std::to_string(file_stat.st_uid);
        }

        std::string group;
        if (gr) {
            group = gr->gr_name;
        } else {
            group = std::to_string(file_stat.st_gid);
        }

        std::string ownership = owner + ":" + group;

        // Calculate checksum
        std::string checksum = generate_file_checksum(full_file_path);
        if (checksum.empty()) {
            dpm_log(LOG_ERROR, ("Failed to generate checksum for: " + full_file_path.string()).c_str());
            continue;
        }

        // By default, mark new files as controlled ('C')
        char control_designation = 'C';

        // Write new line to the temporary file
        temp_manifest_file << control_designation << " "
                          << checksum << " "
                          << perms << " "
                          << ownership << " "
                          << "/" << file_path.string() << std::endl;

        new_files++;
    }

    temp_manifest_file.close();

    // Replace the original file with the temporary file
    try {
        std::filesystem::rename(temp_manifest_path, manifest_path);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to update manifest file: " + std::string(e.what())).c_str());
        std::filesystem::remove(temp_manifest_path);
        return 1;
    }

    // Log results
    if (updated_files > 0) {
        dpm_log(LOG_INFO, ("Updated checksums for " + std::to_string(updated_files) + " existing file(s).").c_str());
    }
    if (new_files > 0) {
        dpm_log(LOG_INFO, ("Added " + std::to_string(new_files) + " new file(s) to manifest.").c_str());
    }

    return 0;
}

static int generate_contents_manifest(
    const std::string& package_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture,
    bool force
) {
    dpm_log(LOG_INFO, ("Generating content manifest for: " + package_dir).c_str());

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
    bool refresh = false;
    bool verbose = false;
    bool show_help = false;
    std::string package_dir = "";

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
        } else if ((arg == "-p" || arg == "--package-dir") && i + 1 < argc) {
            package_dir = argv[i + 1];
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

    // Call the appropriate function based on the refresh flag
    if (refresh) {
        int result = refresh_contents_manifest(package_dir, force);
        if (result != 0) {
            dpm_log(LOG_ERROR, "Failed to refresh contents manifest.");
            return result;
        }
        dpm_log(LOG_INFO, "Contents manifest refreshed successfully.");
        return 0;
    } else {
        bool success = generate_contents_manifest(std::filesystem::path(package_dir));
        if (!success) {
            dpm_log(LOG_ERROR, "Failed to generate contents manifest.");
            return 1;
        }
        dpm_log(LOG_INFO, "Contents manifest generated successfully.");
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
    dpm_log(LOG_INFO, "DPM Build Module - Creates DPM packages.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Available commands:");
    dpm_log(LOG_INFO, "  stage      - Stage a new DPM package directory");
    dpm_log(LOG_INFO, "  manifest   - Generate or refresh package manifest");
    dpm_log(LOG_INFO, "  sign       - Sign a package or package stage directory");
    dpm_log(LOG_INFO, "  seal       - Seal a package stage directory into final format");
    dpm_log(LOG_INFO, "  unseal     - Unseal a package back to stage format");
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
    dpm_log(LOG_INFO, "  -r, --refresh             Refresh existing manifest (default: generate new)");
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

int cmd_sign_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "Usage: dpm build sign [options]");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Sign a DPM package or package stage directory using GPG.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -k, --key-id ID          GPG key ID or email to use for signing (required)");
    dpm_log(LOG_INFO, "  -s, --stage DIR          Package stage directory to sign");
    dpm_log(LOG_INFO, "  -p, --package FILE       Package file to sign");
    dpm_log(LOG_INFO, "  -f, --force              Force signing even if warnings occur");
    dpm_log(LOG_INFO, "  -v, --verbose            Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help               Display this help message");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Either --stage or --package must be specified, but not both.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Examples:");
    dpm_log(LOG_INFO, "  dpm build sign --key-id=\"user@example.com\" --stage=./my-package-1.0.x86_64");
    dpm_log(LOG_INFO, "  dpm build sign --key-id=\"AB123CD456\" --package=./my-package-1.0.x86_64.dpm");
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
        dpm_log(LOG_ERROR, "Input path is required (--input/-i)");
        return cmd_unseal_help(argc, argv);
    }

    // Check for invalid option combinations
    if (components_mode && !output_dir.empty()) {
        dpm_log(LOG_ERROR, "Output directory (-o/--output) cannot be specified in components mode (-c/--components)");
        return cmd_unseal_help(argc, argv);
    }

    // Expand path if needed
    input_path = expand_path(input_path);

    // Check if input path exists
    if (!std::filesystem::exists(input_path)) {
        dpm_log(LOG_ERROR, ("Input path does not exist: " + input_path).c_str());
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
            dpm_log(LOG_ERROR, ("Input path must be a directory in components mode: " + input_path).c_str());
            return 1;
        }

        // Call unseal_stage_components with just the input path
        return unseal_stage_components(input_path);
    } else {
        // We're unsealing a package file
        if (std::filesystem::is_directory(input_path)) {
            dpm_log(LOG_ERROR, ("Input path must be a file when not in components mode: " + input_path).c_str());
            return 1;
        }

        // Call unseal_package
        return unseal_package(input_path, output_dir, force);
    }
}

int cmd_unseal_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "Usage: dpm build unseal [options]");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Unseals a DPM package file or package stage components.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -i, --input PATH       Path to package file or stage directory (required)");
    dpm_log(LOG_INFO, "  -o, --output DIR       Directory to extract package to (optional, package mode only)");
    dpm_log(LOG_INFO, "  -c, --components       Component mode: unseal components in a stage directory");
    dpm_log(LOG_INFO, "                         Without this flag, input is treated as a package file");
    dpm_log(LOG_INFO, "  -f, --force            Force unsealing even if warnings occur or directory exists");
    dpm_log(LOG_INFO, "  -v, --verbose          Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help             Display this help message");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Examples:");
    dpm_log(LOG_INFO, "  # Unseal a package file to a directory:");
    dpm_log(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64.dpm");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "  # Unseal a package file to a specific directory:");
    dpm_log(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64.dpm --output=./extract");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "  # Unseal components in a stage directory:");
    dpm_log(LOG_INFO, "  dpm build unseal --input=./my-package-1.0.x86_64 --components");
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
        dpm_log(LOG_ERROR, "Stage directory is required (--stage/-s)");
        return cmd_seal_help(argc, argv);
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

    // Call the appropriate sealing function based on the finalize flag
    if (finalize) {
        return seal_final_package(stage_dir, output_dir, force);
    } else {
        return seal_stage_components(stage_dir, force);
    }
}

int cmd_seal_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "Usage: dpm build seal [options]");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Seals a package stage directory by replacing contents, metadata,");
    dpm_log(LOG_INFO, "hooks, and signatures directories with gzipped tarballs.");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Options:");
    dpm_log(LOG_INFO, "  -s, --stage DIR         Package stage directory to seal (required)");
    dpm_log(LOG_INFO, "  -o, --output DIR        Output directory for the finalized package (optional)");
    dpm_log(LOG_INFO, "  -f, --force             Force sealing even if warnings occur");
    dpm_log(LOG_INFO, "  -z, --finalize          Also compress the entire stage as a final package");
    dpm_log(LOG_INFO, "  -v, --verbose           Enable verbose output");
    dpm_log(LOG_INFO, "  -h, --help              Display this help message");
    dpm_log(LOG_INFO, "");
    dpm_log(LOG_INFO, "Examples:");
    dpm_log(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64");
    dpm_log(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64 --finalize");
    dpm_log(LOG_INFO, "  dpm build seal --stage=./my-package-1.0.x86_64 --finalize --output=/tmp");
    return 0;
}