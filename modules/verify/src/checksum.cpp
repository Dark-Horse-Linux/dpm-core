/**
 * @file checksum.cpp
 * @brief Implementation of package checksum verification functions
 *
 * Implements functions for verifying checksums of DPM package components.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "checksum.hpp"
#include <fstream>
#include <sstream>
#include <dlfcn.h>

int checksum_verify_contents_digest(const std::string& stage_dir, void* build_module) {
    dpm_log(LOG_INFO, "Verifying contents manifest digest...");
    std::filesystem::path manifest_file = std::filesystem::path(stage_dir) / "metadata" / "CONTENTS_MANIFEST_DIGEST";

    if (!std::filesystem::exists(manifest_file)) {
        dpm_log(LOG_ERROR, "CONTENTS_MANIFEST_DIGEST file not found");
        return 1;
    }

    // Get the generate_file_checksum function from the build module
    typedef std::string (*FileChecksumFunc)(const std::filesystem::path&);
    dlerror(); // Clear any previous error
    FileChecksumFunc generate_checksum = (FileChecksumFunc)dlsym(build_module, "generate_file_checksum");

    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        dpm_log(LOG_ERROR, ("Failed to find generate_file_checksum function: " +
                           std::string(dlsym_error)).c_str());
        return 1;
    }

    try {
        std::ifstream manifest(manifest_file);
        if (!manifest.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open manifest file: " + manifest_file.string()).c_str());
            return 1;
        }

        std::string line;
        int line_number = 0;
        int errors = 0;

        while (std::getline(manifest, line)) {
            line_number++;

            // Skip empty lines
            if (line.empty()) continue;

            // Parse the line: control_designation checksum permissions owner:group /path/to/file
            std::istringstream iss(line);
            char control_designation;
            std::string checksum, permissions, ownership, file_path;

            if (!(iss >> control_designation >> checksum >> permissions >> ownership)) {
                dpm_log(LOG_WARN, ("Malformed manifest line " + std::to_string(line_number) +
                                  ": " + line).c_str());
                continue;
            }

            // Get the rest of the line as the file path
            std::getline(iss >> std::ws, file_path);

            if (file_path.empty()) {
                dpm_log(LOG_WARN, ("Missing file path in manifest line " +
                                  std::to_string(line_number)).c_str());
                continue;
            }

            // Remove leading slash if present
            if (file_path[0] == '/') {
                file_path = file_path.substr(1);
            }

            // Build the full path to the file in the contents directory
            std::filesystem::path full_file_path = std::filesystem::path(stage_dir) / "contents" / file_path;

            // Check if the file exists
            if (!std::filesystem::exists(full_file_path)) {
                dpm_log(LOG_ERROR, ("File not found: " + full_file_path.string()).c_str());
                errors++;
                continue;
            }

            // Calculate the checksum of the file and compare with the manifest
            std::string calculated_checksum = generate_checksum(full_file_path);

            if (calculated_checksum.empty()) {
                dpm_log(LOG_ERROR, ("Failed to calculate checksum for: " +
                                   full_file_path.string()).c_str());
                errors++;
                continue;
            }

            if (calculated_checksum != checksum) {
                dpm_log(LOG_ERROR, ("Checksum mismatch for " + full_file_path.string() +
                                   "\n  Expected: " + checksum +
                                   "\n  Actual:   " + calculated_checksum).c_str());
                errors++;
            }
        }

        manifest.close();

        if (errors > 0) {
            dpm_log(LOG_ERROR, (std::to_string(errors) + " checksum errors found in contents manifest").c_str());
            return 1;
        }

        dpm_log(LOG_INFO, "Contents manifest checksum verification successful");
        return 0;

    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Error processing contents manifest: " + std::string(e.what())).c_str());
        return 1;
    }
}

int checksum_verify_hooks_digest(const std::string& stage_dir, void* build_module) {
    dpm_log(LOG_INFO, "Verifying hooks digest...");
    std::filesystem::path hooks_digest_file = std::filesystem::path(stage_dir) / "metadata" / "HOOKS_DIGEST";

    if (!std::filesystem::exists(hooks_digest_file)) {
        dpm_log(LOG_ERROR, "HOOKS_DIGEST file not found");
        return 1;
    }

    // Get the generate_file_checksum function from the build module
    typedef std::string (*FileChecksumFunc)(const std::filesystem::path&);
    dlerror(); // Clear any previous error
    FileChecksumFunc generate_checksum = (FileChecksumFunc)dlsym(build_module, "generate_file_checksum");

    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        dpm_log(LOG_ERROR, ("Failed to find generate_file_checksum function: " +
                           std::string(dlsym_error)).c_str());
        return 1;
    }

    try {
        std::ifstream hooks_digest(hooks_digest_file);
        if (!hooks_digest.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open hooks digest file: " +
                               hooks_digest_file.string()).c_str());
            return 1;
        }

        std::string line;
        int errors = 0;

        while (std::getline(hooks_digest, line)) {
            // Skip empty lines
            if (line.empty()) continue;

            // Parse the line: checksum filename
            std::istringstream iss(line);
            std::string checksum, filename;

            if (!(iss >> checksum >> filename)) {
                dpm_log(LOG_WARN, ("Malformed hooks digest line: " + line).c_str());
                continue;
            }

            // Build the full path to the hook file
            std::filesystem::path hook_path = std::filesystem::path(stage_dir) / "hooks" / filename;

            // Check if the file exists
            if (!std::filesystem::exists(hook_path)) {
                dpm_log(LOG_ERROR, ("Hook file not found: " + hook_path.string()).c_str());
                errors++;
                continue;
            }

            // Calculate the checksum of the hook file
            std::string calculated_checksum = generate_checksum(hook_path);

            if (calculated_checksum.empty()) {
                dpm_log(LOG_ERROR, ("Failed to calculate checksum for: " +
                                   hook_path.string()).c_str());
                errors++;
                continue;
            }

            if (calculated_checksum != checksum) {
                dpm_log(LOG_ERROR, ("Checksum mismatch for " + hook_path.string() +
                                   "\n  Expected: " + checksum +
                                   "\n  Actual:   " + calculated_checksum).c_str());
                errors++;
            }
        }

        hooks_digest.close();

        if (errors > 0) {
            dpm_log(LOG_ERROR, (std::to_string(errors) + " checksum errors found in hooks digest").c_str());
            return 1;
        }

        dpm_log(LOG_INFO, "Hooks digest checksum verification successful");
        return 0;

    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Error processing hooks digest: " + std::string(e.what())).c_str());
        return 1;
    }
}

int checksum_verify_package_digest(const std::string& stage_dir, void* build_module) {
    dpm_log(LOG_INFO, "Verifying package digest...");
    std::filesystem::path metadata_dir = std::filesystem::path(stage_dir) / "metadata";
    std::filesystem::path package_digest_file = metadata_dir / "PACKAGE_DIGEST";
    std::filesystem::path manifest_file = metadata_dir / "CONTENTS_MANIFEST_DIGEST";
    std::filesystem::path hooks_digest_file = metadata_dir / "HOOKS_DIGEST";

    if (!std::filesystem::exists(package_digest_file)) {
        dpm_log(LOG_ERROR, "PACKAGE_DIGEST file not found");
        return 1;
    }

    if (!std::filesystem::exists(manifest_file)) {
        dpm_log(LOG_ERROR, "CONTENTS_MANIFEST_DIGEST file not found");
        return 1;
    }

    if (!std::filesystem::exists(hooks_digest_file)) {
        dpm_log(LOG_ERROR, "HOOKS_DIGEST file not found");
        return 1;
    }

    // Get the checksum functions from the build module
    typedef std::string (*FileChecksumFunc)(const std::filesystem::path&);
    typedef std::string (*StringChecksumFunc)(const std::string&);

    dlerror(); // Clear any previous error
    FileChecksumFunc generate_file_checksum = (FileChecksumFunc)dlsym(build_module, "generate_file_checksum");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        dpm_log(LOG_ERROR, ("Failed to find generate_file_checksum function: " +
                           std::string(dlsym_error)).c_str());
        return 1;
    }

    dlerror(); // Clear any previous error
    StringChecksumFunc generate_string_checksum = (StringChecksumFunc)dlsym(build_module, "generate_string_checksum");
    dlsym_error = dlerror();
    if (dlsym_error) {
        dpm_log(LOG_ERROR, ("Failed to find generate_string_checksum function: " +
                           std::string(dlsym_error)).c_str());
        return 1;
    }

    // Read the package digest from the file
    std::string package_digest;
    try {
        std::ifstream digest_file(package_digest_file);
        if (!digest_file.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open digest file: " + package_digest_file.string()).c_str());
            return 1;
        }
        std::getline(digest_file, package_digest);
        digest_file.close();
    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Error reading package digest: " + std::string(e.what())).c_str());
        return 1;
    }

    // Calculate checksums of the digest files
    std::string contents_manifest_checksum = generate_file_checksum(manifest_file);
    std::string hooks_digest_checksum = generate_file_checksum(hooks_digest_file);

    if (contents_manifest_checksum.empty() || hooks_digest_checksum.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate checksums for digest files");
        return 1;
    }

    // Combine checksums and calculate package digest
    std::string combined_checksums = contents_manifest_checksum + hooks_digest_checksum;
    std::string calculated_package_digest = generate_string_checksum(combined_checksums);

    if (calculated_package_digest.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate package digest");
        return 1;
    }

    // Compare with the stored package digest
    if (calculated_package_digest != package_digest) {
        dpm_log(LOG_ERROR, ("Package digest mismatch\n  Expected: " + package_digest +
                           "\n  Actual:   " + calculated_package_digest).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, "Package digest verification successful");
    return 0;
}