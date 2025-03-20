/**
 * @file metadata.cpp
 * @brief Implementation of DPM package metadata functions
 *
 * Implements functions for creating and manipulating DPM package metadata.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "metadata.hpp"

bool metadata_generate_new(
    const std::filesystem::path& package_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture
) {
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
            "AUTHOR",
            "MAINTAINER",
            "DEPENDENCIES",
            "DESCRIPTION",
            "CONTENTS_MANIFEST_DIGEST",
            "LICENSE",
            "PACKAGE_DIGEST",
            "HOOKS_DIGEST",
            "PROVIDES",
            "REPLACES",
            "SOURCE",
            "CHANGELOG"
        };

        for (const auto& file_name : metadata_files) {
            std::ofstream metadata_file(metadata_dir / file_name);
            metadata_file.close();
        }

        dpm_log(LOG_INFO, "Created metadata files");

        // Update the contents manifest
        if (!update_contents_manifest(package_dir)) {
            dpm_log(LOG_ERROR, "Failed to update contents manifest");
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to create metadata files: " + std::string(e.what())).c_str());
        return false;
    }
}

/**
 * @brief Updates the contents manifest file for a package
 *
 * Creates the CONTENTS_MANIFEST_DIGEST file by scanning the contents directory
 * and generating a line for each file with control designation,
 * checksum, permissions, ownership, and path information.
 *
 * @param package_dir Root directory of the package being staged
 * @return true if manifest generation was successful, false otherwise
 */
bool update_contents_manifest(const std::filesystem::path& package_dir)
{
    try {
        std::filesystem::path contents_dir = package_dir / "contents";
        std::filesystem::path manifest_path = package_dir / "metadata" / "CONTENTS_MANIFEST_DIGEST";

        // Log which hash algorithm is being used
        std::string hash_algorithm = get_configured_hash_algorithm();
        dpm_log(LOG_INFO, ("Generating contents manifest using " + hash_algorithm + " checksums...").c_str());

        // Open manifest file for writing
        std::ofstream manifest_file(manifest_path);
        if (!manifest_file.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open manifest file for writing: " + manifest_path.string()).c_str());
            return false;
        }

        // Process each file in the contents directory recursively
        for (const auto& entry : std::filesystem::recursive_directory_iterator(contents_dir)) {
            // Skip directories, we only need to record files
            if (std::filesystem::is_directory(entry)) {
                continue;
            }

            // Get file information
            std::filesystem::path file_path = entry.path();
            std::filesystem::path relative_path = std::filesystem::relative(file_path, contents_dir);
            std::string absolute_path = "/" + relative_path.string();  // Add leading slash

            // Get file stats for permissions
            struct stat file_stat;
            if (stat(file_path.c_str(), &file_stat) != 0) {
                dpm_log(LOG_FATAL, ("Failed to get file stats for: " + file_path.string()).c_str());
                return false;
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

            // Calculate file checksum using the configured algorithm
            std::string checksum = generate_file_checksum(file_path);
            if (checksum.empty()) {
                dpm_log(LOG_FATAL, ("Failed to generate checksum for: " + file_path.string()).c_str());
                return false;
            }

            // By default, mark all files as controlled ('C')
            char control_designation = 'C';

            // Write the manifest entry
            // Format: control_designation checksum permissions owner:group /absolute/path
            manifest_file << control_designation << " "
                          << checksum << " "
                          << perms << " "
                          << ownership << " "
                          << absolute_path << "\n";
        }

        manifest_file.close();
        dpm_log(LOG_INFO, "Contents manifest generated successfully");
        return true;
    }
    catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to generate contents manifest: " + std::string(e.what())).c_str());
        return false;
    }
}