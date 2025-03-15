/**
 * @file package_staging.cpp
 * @brief Implementation of DPM package staging functions
 *
 * Implements functions for staging DPM packages according to specification.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "package_staging.hpp"

int build_package_stage(
    const std::string& output_dir,
    const std::string& contents_dir,
    const std::string& hooks_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture,
    const std::string& os,
    bool force
) {
    // Log start of package staging
    dpm_log(LOG_INFO, "Starting package staging...");

    // Create the basic package structure in the output directory
    std::string package_dirname = package_name + "-" + package_version + "." + os + "." + architecture;
    std::filesystem::path package_dir = std::filesystem::path(output_dir) / package_dirname;

    // Check if the package directory already exists
    if (std::filesystem::exists(package_dir)) {
        if (!force) {
            dpm_log(LOG_ERROR, ("Package directory already exists: " + package_dir.string() +
                      ". Use --force to overwrite.").c_str());
            return 1;
        }

        // If force flag is set, try to remove the existing directory
        dpm_log(LOG_WARN, ("Removing existing package directory: " + package_dir.string()).c_str());
        try {
            std::filesystem::remove_all(package_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to remove existing directory: " + std::string(e.what())).c_str());
            return 1;
        }
    }

    // Create the package directory structure
    try {
        // Create main package directory
        std::filesystem::create_directory(package_dir);

        // Create subdirectories
        std::filesystem::create_directory(package_dir / "contents");
        std::filesystem::create_directory(package_dir / "hooks");
        std::filesystem::create_directory(package_dir / "metadata");
        std::filesystem::create_directory(package_dir / "signatures");

        dpm_log(LOG_INFO, ("Created package directory structure at: " + package_dir.string()).c_str());
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create package directory structure: " + std::string(e.what())).c_str());
        return 1;
    }

    // Copy contents from source directory to package contents directory
    try {
        std::filesystem::path contents_source = std::filesystem::path(contents_dir);
        std::filesystem::path contents_dest = package_dir / "contents";

        dpm_log(LOG_INFO, ("Copying contents from: " + contents_source.string() +
                 " to: " + contents_dest.string()).c_str());

        // If the contents source is a directory, copy its contents
        if (std::filesystem::is_directory(contents_source)) {
            for (const auto& entry : std::filesystem::directory_iterator(contents_source)) {
                // Get the relative path from the source directory
                std::filesystem::path relative = entry.path().lexically_relative(contents_source);
                std::filesystem::path dest_path = contents_dest / relative;

                if (entry.is_directory()) {
                    std::filesystem::create_directories(dest_path);
                    // Copy the directory contents recursively
                    for (const auto& subentry : std::filesystem::recursive_directory_iterator(entry)) {
                        std::filesystem::path subrelative = subentry.path().lexically_relative(contents_source);
                        std::filesystem::path subdest_path = contents_dest / subrelative;

                        if (subentry.is_directory()) {
                            std::filesystem::create_directories(subdest_path);
                        } else {
                            std::filesystem::copy_file(subentry.path(), subdest_path,
                                                      std::filesystem::copy_options::overwrite_existing);
                        }
                    }
                } else {
                    std::filesystem::copy_file(entry.path(), dest_path,
                                              std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
        // If the contents source is a file (like a tarball), just copy it
        else if (std::filesystem::is_regular_file(contents_source)) {
            std::filesystem::path dest_path = contents_dest / contents_source.filename();
            std::filesystem::copy_file(contents_source, dest_path,
                                      std::filesystem::copy_options::overwrite_existing);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to copy contents: " + std::string(e.what())).c_str());
        return 1;
    }

    // Copy hooks if provided
    if (!hooks_dir.empty()) {
        try {
            std::filesystem::path hooks_source = std::filesystem::path(hooks_dir);
            std::filesystem::path hooks_dest = package_dir / "hooks";

            dpm_log(LOG_INFO, ("Copying hooks from: " + hooks_source.string() +
                     " to: " + hooks_dest.string()).c_str());

            // If hooks source is a directory, copy its contents
            if (std::filesystem::is_directory(hooks_source)) {
                for (const auto& entry : std::filesystem::directory_iterator(hooks_source)) {
                    // Get the relative path from the source directory
                    std::filesystem::path relative = entry.path().lexically_relative(hooks_source);
                    std::filesystem::path dest_path = hooks_dest / relative;

                    if (entry.is_directory()) {
                        std::filesystem::create_directories(dest_path);
                    } else {
                        std::filesystem::copy_file(entry.path(), dest_path,
                                                  std::filesystem::copy_options::overwrite_existing);

                        // Make hook files executable
                        chmod(dest_path.c_str(), 0755);
                    }
                }
            }
            // If hooks source is a file, just copy it
            else if (std::filesystem::is_regular_file(hooks_source)) {
                std::filesystem::path dest_path = hooks_dest / hooks_source.filename();
                std::filesystem::copy_file(hooks_source, dest_path,
                                          std::filesystem::copy_options::overwrite_existing);

                // Make hook file executable
                chmod(dest_path.c_str(), 0755);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to copy hooks: " + std::string(e.what())).c_str());
            return 1;
        }
    } else {
        // If no hooks directory provided, create empty hook script templates
        std::vector<std::string> hook_names = {
            "PRE-INSTALL", "PRE-INSTALL_ROLLBACK",
            "POST-INSTALL", "POST-INSTALL_ROLLBACK",
            "PRE-UPDATE", "PRE-UPDATE_ROLLBACK",
            "POST-UPDATE", "POST-UPDATE_ROLLBACK",
            "PRE-REMOVE", "PRE-REMOVE_ROLLBACK",
            "POST-REMOVE", "POST-REMOVE_ROLLBACK"
        };

        dpm_log(LOG_INFO, "Creating empty hook templates");

        for (const auto& hook_name : hook_names) {
            std::filesystem::path hook_path = package_dir / "hooks" / hook_name;
            std::ofstream hook_file(hook_path);

            if (hook_file.is_open()) {
                hook_file << "#!/bin/sh\n";
                hook_file << "# " << hook_name << " hook for " << package_name << "\n";
                hook_file << "# This is a template. Modify as needed.\n\n";
                hook_file << "# Exit with non-zero status to indicate failure\n";
                hook_file << "exit 0\n";
                hook_file.close();

                // Make the hook file executable
                chmod(hook_path.c_str(), 0755);
            } else {
                dpm_log(LOG_ERROR, ("Failed to create hook file: " + hook_path.string()).c_str());
            }
        }
    }

    // Create basic metadata files
    try {
        std::filesystem::path metadata_dir = package_dir / "metadata";

        // Create NAME file
        {
            std::ofstream name_file(metadata_dir / "NAME");
            if (name_file.is_open()) {
                name_file << package_name;
                name_file.close();
            }
        }

        // Create VERSION file
        {
            std::ofstream version_file(metadata_dir / "VERSION");
            if (version_file.is_open()) {
                version_file << package_version;
                version_file.close();
            }
        }

        // Create ARCHITECTURE file
        {
            std::ofstream arch_file(metadata_dir / "ARCHITECTURE");
            if (arch_file.is_open()) {
                arch_file << architecture;
                arch_file.close();
            }
        }

        // Create empty placeholder files for other metadata
        std::vector<std::string> metadata_files = {
            "AUTHOR", "MAINTAINER", "DEPENDENCIES", "DESCRIPTION",
            "CONTENTS_MANIFEST_DIGEST", "LICENSE", "PACKAGE_DIGEST",
            "HOOKS_DIGEST", "PROVIDES", "REPLACES", "SOURCE", "CHANGELOG"
        };

        for (const auto& file_name : metadata_files) {
            std::ofstream metadata_file(metadata_dir / file_name);
            metadata_file.close();
        }

        dpm_log(LOG_INFO, "Created metadata files");
    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to create metadata files: " + std::string(e.what())).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, "Package staging completed successfully");
    dpm_log(LOG_INFO, ("Package staged at: " + package_dir.string()).c_str());
    dpm_log(LOG_INFO, "Next steps:");
    dpm_log(LOG_INFO, "1. Edit metadata files to provide package information");
    dpm_log(LOG_INFO, "2. Update CONTENTS_MANIFEST_DIGEST to mark controlled/non-controlled files");
    dpm_log(LOG_INFO, "3. Customize hook scripts as needed");

    return 0;
}