/**
* @file metadata.hpp
 * @brief Functions for handling DPM package stage metadata
 *
 * Defines functions for creating and manipulating metadata for DPM package stages.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <map>

#include <dpmdk/include/CommonModuleAPI.hpp>
#include "checksums.hpp"

// generates the initial entries for the stage - does not populate data!
bool metadata_generate_skeleton(const std::filesystem::path& stage_dir);

// sets values in metadata files
bool metadata_set_simple_value(const std::filesystem::path& stage_dir, const std::string& key, const std::string& value);

// sets initial known values in metadata
bool metadata_set_initial_known_values(
    const std::filesystem::path& stage_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture
);

/**
 * @brief Updates the contents manifest file for a package stage
 *
 * Creates the CONTENTS_MANIFEST_DIGEST file by scanning the contents directory
 * and generating a line for each file with control designation,
 * checksum, permissions, ownership, and path information.
 *
 * @param package_dir Root directory of the package stage
 * @return true if contents manifest generation was successful, false otherwise
 */
bool metadata_generate_contents_manifest_digest(const std::filesystem::path& package_dir);

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
int metadata_refresh_contents_manifest_digest(const std::string& stage_dir, bool force);

/**
 * @brief Generates the HOOKS_DIGEST file for a package stage
 *
 * Creates the HOOKS_DIGEST file by scanning the hooks directory
 * and generating a line for each hook file with its checksum and filename.
 *
 * @param stage_dir Root directory of the package stage
 * @return true if hooks digest generation was successful, false otherwise
 */
bool metadata_generate_hooks_digest(const std::filesystem::path& stage_dir);

// generates the dynamic entries for the stage
bool metadata_generate_dynamic_files( const std::filesystem::path& stage_dir );

// refreshes the dynamic entries for the stage
bool metadata_refresh_dynamic_files( const std::filesystem::path& stage_dir );

/**
 * @brief Generates basic metadata files for a package stage
 *
 * Creates the necessary metadata files for a package stage with the provided information.
 * Does not generate the CONTENTS_MANIFEST_DIGEST which requires a separate call
 * to update_contents_manifest().
 *
 * @param package_dir Root directory of the package stage
 * @param package_name Name of the package
 * @param package_version Version of the package
 * @param architecture Architecture of the package (e.g., x86_64, aarch64)
 * @return true if metadata generation was successful, false otherwise
 */
bool metadata_generate_new(
    const std::filesystem::path& package_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture
);

/**
 * @brief Generates the PACKAGE_DIGEST for a package stage
 *
 * Creates the PACKAGE_DIGEST by generating checksums of CONTENTS_MANIFEST_DIGEST
 * and HOOKS_DIGEST, concatenating them, and then calculating a checksum of the
 * concatenation. The result is stored in the PACKAGE_DIGEST file.
 *
 * @param stage_dir Root directory of the package stage
 * @return true if package digest generation was successful, false otherwise
 */
bool metadata_generate_package_digest(const std::filesystem::path& stage_dir);