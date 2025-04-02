/**
 * @file verification.cpp
 * @brief Implementation of package verification functions
 *
 * Implements functions for verifying checksums and signatures of DPM packages
 * and package stage directories.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "verification.hpp"


int verify_checksums_package(const std::string& package_path) {
    // Check if the package file exists
    if (!std::filesystem::exists(package_path)) {
        dpm_log(LOG_ERROR, ("Package file not found: " + package_path).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, ("Verifying checksums for package: " + package_path).c_str());

    // Load the build module to access functions
    void* build_module = nullptr;
    int result = check_and_load_build_module(build_module);
    if (result != 0 || build_module == nullptr) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return 1;
    }

    // Create a temporary directory for extraction
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "dpm_verify_tmp";

    // Remove temp directory if it already exists
    if (std::filesystem::exists(temp_dir)) {
        try {
            std::filesystem::remove_all(temp_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to clean up existing temp directory: " + std::string(e.what())).c_str());
            dpm_unload_module(build_module);
            return 1;
        }
    }

    // Create the temp directory
    try {
        std::filesystem::create_directory(temp_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create temp directory: " + std::string(e.what())).c_str());
        dpm_unload_module(build_module);
        return 1;
    }

    // Unseal the package to the temp directory using dpm_execute_symbol
    dpm_log(LOG_INFO, "Unsealing package to temporary directory for verification...");
    std::string output_dir = temp_dir.string();
    bool force = true; // Force overwrite if directory exists

    result = dpm_execute_symbol(build_module, "unseal_package", package_path, output_dir, force);

    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to unseal package for verification");
        dpm_unload_module(build_module);

        // Clean up temp directory
        try {
            std::filesystem::remove_all(temp_dir);
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore cleanup errors
        }

        return 1;
    }

    // Get the extracted stage directory name (package name without .dpm)
    std::string package_filename = std::filesystem::path(package_path).filename().string();
    std::string stage_name = package_filename;

    // Remove .dpm extension if present
    const std::string dpm_extension = ".dpm";
    if (stage_name.ends_with(dpm_extension)) {
        stage_name = stage_name.substr(0, stage_name.length() - dpm_extension.length());
    }

    std::filesystem::path stage_dir = temp_dir / stage_name;

    // Verify checksums in the extracted stage directory
    dpm_log(LOG_INFO, ("Verifying checksums in extracted stage: " + stage_dir.string()).c_str());

    // Now verify the stage directory checksums
    result = verify_checksums_stage(stage_dir.string());

    // Clean up temp directory
    dpm_log(LOG_INFO, "Cleaning up temporary extraction directory...");
    try {
        std::filesystem::remove_all(temp_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_WARN, ("Failed to clean up temp directory: " + std::string(e.what())).c_str());
        // Continue execution - this is just a cleanup warning
    }

    // Clean up module handle
    dpm_unload_module(build_module);

    if (result == 0) {
        dpm_log(LOG_INFO, "Package checksum verification completed successfully");
    } else {
        dpm_log(LOG_ERROR, "Package checksum verification failed");
    }

    return result;
}

int verify_checksums_stage(const std::string& stage_dir) {
    // Check if the stage directory exists
    if (!std::filesystem::exists(stage_dir)) {
        dpm_log(LOG_ERROR, ("Stage directory not found: " + stage_dir).c_str());
        return 1;
    }

    // Check if it's actually a directory
    if (!std::filesystem::is_directory(stage_dir)) {
        dpm_log(LOG_ERROR, ("Path is not a directory: " + stage_dir).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, ("Verifying checksums for stage directory: " + stage_dir).c_str());

    // First, ensure the components are unsealed (uncompressed)
    dpm_log(LOG_INFO, "Ensuring stage components are unsealed...");

    // Load the build module to access functions
    void* build_module = nullptr;
    int result = check_and_load_build_module(build_module);
    if (result != 0 || build_module == nullptr) {
        dpm_log(LOG_ERROR, "Failed to load build module");
        return 1;
    }

    // Use dpm_execute_symbol to call the unseal_stage_components function
    std::filesystem::path stage_path(stage_dir);
    result = dpm_execute_symbol(build_module, "unseal_stage_components", stage_path);

    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to unseal stage components");
        dpm_unload_module(build_module);
        return 1;
    }

    // Verify checksums
    result = checksum_verify_package_digest(stage_dir, build_module);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Package digest verification failed");
        dpm_unload_module(build_module);
        return 1;
    }

    result = checksum_verify_contents_digest(stage_dir, build_module);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Contents manifest verification failed");
        dpm_unload_module(build_module);
        return 1;
    }

    result = checksum_verify_hooks_digest(stage_dir, build_module);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Hooks digest verification failed");
        dpm_unload_module(build_module);
        return 1;
    }

    // Clean up
    dpm_unload_module(build_module);

    dpm_log(LOG_INFO, "All checksums verified successfully");
    return 0;
}

int verify_signature_package(const std::string& package_path) {
    // Check if the package file exists
    if (!std::filesystem::exists(package_path)) {
        dpm_log(LOG_ERROR, ("Package file not found: " + package_path).c_str());
        return 1;
    }

    // Placeholder implementation
    dpm_log(LOG_INFO, ("Verifying signatures for package: " + package_path).c_str());
    dpm_log(LOG_INFO, "Package signature verification not yet implemented");

    return 0;
}


int verify_signature_stage(const std::string& stage_dir) {
    // Check if the stage directory exists
    if (!std::filesystem::exists(stage_dir)) {
        dpm_log(LOG_ERROR, ("Stage directory not found: " + stage_dir).c_str());
        return 1;
    }

    // Check if it's actually a directory
    if (!std::filesystem::is_directory(stage_dir)) {
        dpm_log(LOG_ERROR, ("Path is not a directory: " + stage_dir).c_str());
        return 1;
    }

    // Placeholder implementation
    dpm_log(LOG_INFO, ("Verifying signatures for stage directory: " + stage_dir).c_str());
    dpm_log(LOG_INFO, "Stage directory signature verification not yet implemented");

    return 0;
}