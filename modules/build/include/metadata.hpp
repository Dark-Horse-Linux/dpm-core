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

#include <dpmdk/include/CommonModuleAPI.hpp>
#include "checksums.hpp"

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
bool generate_contents_manifest(const std::filesystem::path& package_dir);

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