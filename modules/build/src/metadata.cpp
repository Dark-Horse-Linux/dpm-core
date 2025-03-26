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

// generates the initial entries for the stage - does not populate data!
bool metadata_generate_skeleton(const std::filesystem::path& stage_dir) {
    // generates empty files, such as when generating a new stage

    // determine the path to the metadata directory
    std::filesystem::path metadata_dir = stage_dir / "metadata";

    // Check if metadata directory exists and is a directory
    if (!std::filesystem::exists(metadata_dir)) {
        dpm_log(LOG_ERROR, ("Metadata directory does not exist: " + metadata_dir.string()).c_str());
        return false;
    }

    if (!std::filesystem::is_directory(metadata_dir)) {
        dpm_log(LOG_ERROR, ("Metadata path exists but is not a directory: " + metadata_dir.string()).c_str());
        return false;
    }

    try {
        // Create empty placeholder files for all metadata
        std::vector<std::string> metadata_files = {
            "NAME",
            "VERSION",
            "ARCHITECTURE",
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
    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to create metadata files: " + std::string(e.what())).c_str());
        return false;
    }

    dpm_log(LOG_INFO, "Metadata skeleton generated.");
    return true;
}

bool metadata_set_simple_value(const std::filesystem::path& stage_dir, const std::string& key, const std::string& value)
{
    // populates single-line entries

    // Determine the path to the metadata file
    std::filesystem::path metadata_file_path = stage_dir / "metadata" / key;

    // Check if the metadata directory exists
    std::filesystem::path metadata_dir = stage_dir / "metadata";
    if (!std::filesystem::exists(metadata_dir)) {
        dpm_log(LOG_ERROR, ("Metadata directory does not exist: " + metadata_dir.string()).c_str());
        return false;
    }

    if (!std::filesystem::is_directory(metadata_dir)) {
        dpm_log(LOG_ERROR, ("Metadata path exists but is not a directory: " + metadata_dir.string()).c_str());
        return false;
    }

    // Check if the metadata file exists
    if (!std::filesystem::exists(metadata_file_path)) {
        dpm_log(LOG_ERROR, ("Metadata file does not exist: " + metadata_file_path.string()).c_str());
        return false;
    }

    try {
        // Open the file for writing (will overwrite existing content)
        std::ofstream file(metadata_file_path);
        if (!file.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open metadata file for writing: " + metadata_file_path.string()).c_str());
            return false;
        }

        // Write the value to the file
        file << value;
        file.close();

        dpm_log(LOG_INFO, ("Set metadata " + key + " to: " + value).c_str());
        return true;
    } catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to write metadata value: " + std::string(e.what())).c_str());
        return false;
    }
}

bool metadata_set_initial_known_values(
    const std::filesystem::path& stage_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture
) {
    std::filesystem::path metadata_dir = stage_dir / "metadata";

    std::filesystem::path name_file = metadata_dir / "NAME";
    std::filesystem::path version_file = metadata_dir / "VERSION";
    std::filesystem::path architecture_file = metadata_dir / "ARCHITECTURE";

    if (!metadata_set_simple_value( stage_dir, "NAME", package_name ))
    {
        dpm_log( LOG_FATAL, "Failed to set 'NAME'." );
        return false;
    }

    if (!metadata_set_simple_value( stage_dir, "VERSION", package_version ))
    {
        dpm_log( LOG_FATAL, "Failed to set 'VERSION'." );
        return false;
    }

    if (!metadata_set_simple_value( stage_dir, "ARCHITECTURE", architecture ))
    {
        dpm_log( LOG_FATAL, "Failed to set 'ARCHITECTURE'." );
        return false;
    }

    return true;
}

bool metadata_generate_contents_manifest_digest(const std::filesystem::path& package_dir)
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
        return true;
    }
    catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to generate contents manifest: " + std::string(e.what())).c_str());
        return false;
    }
}

int metadata_refresh_contents_manifest_digest(const std::string& stage_dir, bool force) {
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

bool metadata_generate_hooks_digest(const std::filesystem::path& stage_dir)
{
    try {
        std::filesystem::path hooks_dir = stage_dir / "hooks";
        std::filesystem::path digest_path = stage_dir / "metadata" / "HOOKS_DIGEST";

        // Check if hooks directory exists
        if (!std::filesystem::exists(hooks_dir)) {
            dpm_log(LOG_ERROR, ("Hooks directory does not exist: " + hooks_dir.string()).c_str());
            return false;
        }

        // Log which hash algorithm is being used
        std::string hash_algorithm = get_configured_hash_algorithm();
        dpm_log(LOG_INFO, ("Generating hooks digest using " + hash_algorithm + " checksums...").c_str());

        // Open digest file for writing
        std::ofstream digest_file(digest_path);
        if (!digest_file.is_open()) {
            dpm_log(LOG_ERROR, ("Failed to open hooks digest file for writing: " + digest_path.string()).c_str());
            return false;
        }

        // Process each file in the hooks directory
        for (const auto& entry : std::filesystem::directory_iterator(hooks_dir)) {
            // Skip directories, we only need to record files
            if (std::filesystem::is_directory(entry)) {
                continue;
            }

            // Get file information
            std::filesystem::path file_path = entry.path();
            std::string filename = entry.path().filename().string();

            // Calculate file checksum using the configured algorithm
            std::string checksum = generate_file_checksum(file_path);
            if (checksum.empty()) {
                dpm_log(LOG_FATAL, ("Failed to generate checksum for: " + file_path.string()).c_str());
                return false;
            }

            // Write the digest entry
            // Format: checksum filename
            digest_file << checksum << " " << filename << "\n";
        }

        digest_file.close();
        dpm_log(LOG_INFO, "Hooks digest generated successfully");
        return true;
    }
    catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to generate hooks digest: " + std::string(e.what())).c_str());
        return false;
    }
}

bool metadata_generate_package_digest(const std::filesystem::path& stage_dir)
{
    try {
        std::filesystem::path contents_manifest_path = stage_dir / "metadata" / "CONTENTS_MANIFEST_DIGEST";
        std::filesystem::path hooks_digest_path = stage_dir / "metadata" / "HOOKS_DIGEST";

        // Check if required files exist
        if (!std::filesystem::exists(contents_manifest_path)) {
            dpm_log(LOG_ERROR, ("CONTENTS_MANIFEST_DIGEST not found: " + contents_manifest_path.string()).c_str());
            return false;
        }

        if (!std::filesystem::exists(hooks_digest_path)) {
            dpm_log(LOG_ERROR, ("HOOKS_DIGEST not found: " + hooks_digest_path.string()).c_str());
            return false;
        }

        // Log which hash algorithm is being used
        std::string hash_algorithm = get_configured_hash_algorithm();
        dpm_log(LOG_INFO, ("Generating package digest using " + hash_algorithm + " checksums...").c_str());

        // Calculate checksums of both files
        std::string contents_manifest_checksum = generate_file_checksum(contents_manifest_path);
        if (contents_manifest_checksum.empty()) {
            dpm_log(LOG_ERROR, ("Failed to generate checksum for: " + contents_manifest_path.string()).c_str());
            return false;
        }

        std::string hooks_digest_checksum = generate_file_checksum(hooks_digest_path);
        if (hooks_digest_checksum.empty()) {
            dpm_log(LOG_ERROR, ("Failed to generate checksum for: " + hooks_digest_path.string()).c_str());
            return false;
        }

        // Concatenate the two checksums
        std::string combined_checksums = contents_manifest_checksum + hooks_digest_checksum;

        // Calculate checksum of the combined string
        std::string package_digest = generate_string_checksum(combined_checksums);
        if (package_digest.empty()) {
            dpm_log(LOG_ERROR, "Failed to generate checksum of combined checksums");
            return false;
        }

        // Set the package digest in the metadata
        if (!metadata_set_simple_value(stage_dir, "PACKAGE_DIGEST", package_digest)) {
            dpm_log(LOG_ERROR, "Failed to set PACKAGE_DIGEST value");
            return false;
        }

        dpm_log(LOG_INFO, "Package digest generated successfully");
        return true;
    }
    catch (const std::exception& e) {
        dpm_log(LOG_ERROR, ("Failed to generate package digest: " + std::string(e.what())).c_str());
        return false;
    }
}

// generates the dynamic entries for the stage
bool metadata_generate_dynamic_files(const std::filesystem::path& stage_dir)
{
    // Generate contents manifest
    dpm_log(LOG_INFO, "Generating contents manifest digest...");
    if (!metadata_generate_contents_manifest_digest(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to generate contents manifest digest");
        return false;
    }

    // Generate hooks digest
    dpm_log(LOG_INFO, "Generating hooks digest...");
    if (!metadata_generate_hooks_digest(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to generate hooks digest");
        return false;
    }

    // Generate package digest
    dpm_log(LOG_INFO, "Generating package digest...");
    if (!metadata_generate_package_digest(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to generate package digest");
        return false;
    }

    dpm_log(LOG_INFO, "Dynamic metadata generation completed successfully");
    return true;
}

// refreshes the dynamic entries for the stage
bool metadata_refresh_dynamic_files(const std::filesystem::path& stage_dir)
{
    // Refresh contents manifest
    dpm_log(LOG_INFO, "Refreshing contents manifest digest...");
    if (metadata_refresh_contents_manifest_digest(stage_dir, false) != 0) {
        dpm_log(LOG_ERROR, "Failed to refresh contents manifest digest");
        return false;
    }

    // Generate hooks digest
    dpm_log(LOG_INFO, "Regenerating hooks digest...");
    if (!metadata_generate_hooks_digest(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to regenerate hooks digest");
        return false;
    }

    // Generate package digest
    dpm_log(LOG_INFO, "Regenerating package digest...");
    if (!metadata_generate_package_digest(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to regenerate package digest");
        return false;
    }

    dpm_log(LOG_INFO, "Dynamic metadata refresh completed successfully");
    return true;
}

bool metadata_generate_new(
    const std::filesystem::path& stage_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture
)
{
    // Step 1: Generate metadata skeleton
    dpm_log(LOG_INFO, "Generating metadata skeleton...");
    if (!metadata_generate_skeleton(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to generate metadata skeleton");
        return false;
    }

    // Step 2: Set initial known values
    dpm_log(LOG_INFO, "Setting initial metadata values...");
    if (!metadata_set_initial_known_values(stage_dir, package_name, package_version, architecture)) {
        dpm_log(LOG_ERROR, "Failed to set initial metadata values");
        return false;
    }

    // Step 3: Generate dynamic files
    dpm_log(LOG_INFO, "Generating dynamic metadata files...");
    if (!metadata_generate_dynamic_files(stage_dir)) {
        dpm_log(LOG_ERROR, "Failed to generate dynamic metadata files");
        return false;
    }

    dpm_log(LOG_INFO, "Metadata generation completed successfully");
    return true;
}