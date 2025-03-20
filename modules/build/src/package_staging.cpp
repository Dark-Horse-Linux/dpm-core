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

// generates a directory for the stage according to naming convention
std::filesystem::path stage_determine_rootdir_path(
    const std::string& output_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture,
    const std::string& os
) {
    std::string package_dirname = package_name + "-" + package_version + "." + os + "." + architecture;
    std::filesystem::path package_dir = std::filesystem::path(output_dir) / package_dirname;

    return package_dir;
}

// check if a directory exists
bool stage_directory_exists( const std::filesystem::path& package_dir )
{
    return std::filesystem::exists(package_dir);
}

bool stage_create_rootdir( const std::filesystem::path& package_dir, bool force )
{
    // Check if the package directory already exists
    if (stage_directory_exists(package_dir))
    {
        if (!force) {
            dpm_log(LOG_ERROR, ("Package directory already exists: " + package_dir.string() +
                ". Use --force to overwrite.").c_str());
            return false;
        }

        // If force flag is set, try to remove the existing directory
        dpm_log(LOG_WARN, ("Removing existing package directory: " + package_dir.string()).c_str());
        try {
            std::filesystem::remove_all(package_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to remove existing directory: " + std::string(e.what())).c_str());
            return false;
        }
    }

    // Create the directory
    try {
        std::filesystem::create_directories(package_dir);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create directory: " + std::string(e.what())).c_str());
        return false;
    }
}

bool stage_create_subdir(const std::filesystem::path& package_dir, const std::string& subdir_name)
{
    try {
        std::filesystem::create_directory(package_dir / subdir_name);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create subdirectory '" + subdir_name +
                 "': " + std::string(e.what())).c_str());
        return false;
    }
}

static bool stage_build_stage_skeleton( const std::filesystem::path& package_dir, bool force )
{
    // Check if the package directory already exists and handle appropriately
    if (!stage_create_rootdir(package_dir, force)) {
        dpm_log( LOG_FATAL, ("Failed to create root directory: " + package_dir.string()).c_str());
        return false;
    }

    // Create subdirectories
    if (!stage_create_subdir(package_dir, "contents"))     { return false; }
    if (!stage_create_subdir(package_dir, "hooks"))        { return false; }
    if (!stage_create_subdir(package_dir, "metadata"))     { return false; }
    if (!stage_create_subdir(package_dir, "signatures"))   { return false; }

    // assume success if we made it to this point
    dpm_log(LOG_INFO, ("Created package directory structure at: " + package_dir.string()).c_str());

    return true;
}


static bool stage_copy_dir( const std::filesystem::path& source_path, const std::filesystem::path& dest_path )
{
    try {
        dpm_log(LOG_INFO, ("Copying from: " + source_path.string() +
                 " to: " + dest_path.string()).c_str());

        // Check if source exists
        if (!std::filesystem::exists(source_path)) {
            dpm_log(LOG_ERROR, ("Source path does not exist: " + source_path.string()).c_str());
            return false;
        }

        // If the contents source is a directory, copy its contents
        if (std::filesystem::is_directory(source_path)) {
            for (const auto& entry : std::filesystem::directory_iterator(source_path)) {
                // Get the relative path from the source directory
                std::filesystem::path relative = entry.path().lexically_relative(source_path);
                std::filesystem::path dest_path_item = dest_path / relative;

                if (entry.is_directory()) {
                    std::filesystem::create_directories(dest_path_item);
                    // Copy the directory contents recursively
                    for (const auto& subentry : std::filesystem::recursive_directory_iterator(entry)) {
                        std::filesystem::path subrelative = subentry.path().lexically_relative(source_path);
                        std::filesystem::path subdest_path = dest_path / subrelative;

                        if (subentry.is_directory()) {
                            std::filesystem::create_directories(subdest_path);
                        } else {
                            std::filesystem::copy_file(subentry.path(), subdest_path,
                                                     std::filesystem::copy_options::overwrite_existing);
                        }
                    }
                } else {
                    std::filesystem::copy_file(entry.path(), dest_path_item,
                                             std::filesystem::copy_options::overwrite_existing);
                }
            }
        } else {
            dpm_log(LOG_ERROR, ("Source is not a directory: " + source_path.string()).c_str());
            return false;
        }

        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to copy: " + std::string(e.what())).c_str());
        return false;
    }
}

static bool stage_populate_contents(
    const std::filesystem::path& package_dir,
    const std::string& contents_dir
) {
    std::filesystem::path contents_source = std::filesystem::path(contents_dir);
    std::filesystem::path contents_dest = package_dir / "contents";

    if (!stage_copy_dir(contents_source, contents_dest))
    {
        dpm_log( LOG_FATAL, "Failed to copy the contents directory to the package stage.  Exiting." );
        return false;
    }

    return true;
}

static bool stage_populate_hooks(
    const std::filesystem::path& package_dir,
    const std::string& hooks_dir,
    const std::string& package_name
) {
    // Define the required hook names
    std::vector<std::string> hook_names = {
        "PRE-INSTALL", "PRE-INSTALL_ROLLBACK",
        "POST-INSTALL", "POST-INSTALL_ROLLBACK",
        "PRE-UPDATE", "PRE-UPDATE_ROLLBACK",
        "POST-UPDATE", "POST-UPDATE_ROLLBACK",
        "PRE-REMOVE", "PRE-REMOVE_ROLLBACK",
        "POST-REMOVE", "POST-REMOVE_ROLLBACK"
    };

    // Copy hooks if provided
    if (!hooks_dir.empty())
    {
        std::filesystem::path hooks_source = std::filesystem::path(hooks_dir);
        std::filesystem::path hooks_dest = package_dir / "hooks";

        // Validate the hooks directory before copying
        if (std::filesystem::exists(hooks_source) && std::filesystem::is_directory(hooks_source)) {
            // Check if all required hooks are present
            for (const auto& hook_name : hook_names) {
                std::filesystem::path hook_path = hooks_source / hook_name;
                if (!std::filesystem::exists(hook_path)) {
                    dpm_log(LOG_ERROR, ("Missing required hook file: " + hook_name).c_str());
                    return false;
                }
            }

            // Check for unexpected files or directories
            for (const auto& entry : std::filesystem::directory_iterator(hooks_source)) {
                std::string filename = entry.path().filename().string();

                // Check if entry is a directory and reject it
                if (std::filesystem::is_directory(entry)) {
                    dpm_log(LOG_ERROR, ("Unexpected directory in hooks directory: " + filename).c_str());
                    return false;
                }

                // Check if the file is a valid hook
                bool is_valid_hook = false;
                for (const auto& hook_name : hook_names) {
                    if (filename == hook_name) {
                        is_valid_hook = true;
                        break;
                    }
                }

                if (!is_valid_hook) {
                    dpm_log(LOG_ERROR, ("Unexpected file in hooks directory: " + filename).c_str());
                    return false;
                }
            }
        } else {
            dpm_log(LOG_ERROR, ("Hooks directory does not exist or is not a directory: " + hooks_source.string()).c_str());
            return false;
        }

        // Now copy the directory after validation
        if (!stage_copy_dir(hooks_source, hooks_dest))
        {
            dpm_log(LOG_FATAL, "Failed to copy the hooks directory to the package stage. Exiting.");
            return false;
        }

        // Make hook files executable
        for (const auto& entry : std::filesystem::recursive_directory_iterator(hooks_dest)) {
            if (!entry.is_directory()) {
                chmod(entry.path().c_str(), 0755);
            }
        }
    } else {
        // If no hooks directory provided, create empty hook script templates
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
                return false;
            }
        }
    }

    return true;
}

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

    // Generate package directory path
    std::filesystem::path package_dir = stage_determine_rootdir_path(
        output_dir, package_name, package_version, architecture, os
    );

    // Build the package skeleton
    if (!stage_build_stage_skeleton(package_dir, force))
    {
        return 1;
    }

    // copy the contents dir to the contents part of the package stage
    if (!stage_populate_contents(package_dir, contents_dir))
    {
        return 1;
    }

    // copy the supplied hooks or create a new blank hooks dir in the stage
    if (!stage_populate_hooks(package_dir, hooks_dir, package_name))
    {
        return 1;
    }

    // Populate metadata files
    if (!metadata_generate_new(package_dir, package_name, package_version, architecture))
    {
        return 1;
    }

    // Update the contents manifest
    if (!update_contents_manifest(package_dir))
    {
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