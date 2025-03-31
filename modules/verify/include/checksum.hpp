/**
* @file checksum.hpp
 * @brief Header file for package checksum verification functions
 *
 * Defines functions for verifying checksums of DPM package components.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <filesystem>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include "checksum.hpp"

/**
 * @brief Verify the CONTENTS_MANIFEST_DIGEST file
 *
 * Compares checksums in manifest with actual file checksums
 *
 * @param stage_dir Path to the stage directory
 * @param build_module Handle to the loaded build module
 * @return 0 on success, non-zero on failure
 */
int checksum_verify_contents_digest(const std::string& stage_dir, void* build_module);

/**
 * @brief Verify the HOOKS_DIGEST file
 *
 * Compares checksums in hooks digest with actual file checksums
 *
 * @param stage_dir Path to the stage directory
 * @param build_module Handle to the loaded build module
 * @return 0 on success, non-zero on failure
 */
int checksum_verify_hooks_digest(const std::string& stage_dir, void* build_module);

/**
 * @brief Verify the PACKAGE_DIGEST file
 *
 * Calculates the combined checksum of CONTENTS_MANIFEST_DIGEST and HOOKS_DIGEST
 * and compares it with the value in PACKAGE_DIGEST
 *
 * @param stage_dir Path to the stage directory
 * @param build_module Handle to the loaded build module
 * @return 0 on success, non-zero on failure
 */
int checksum_verify_package_digest(const std::string& stage_dir, void* build_module);
