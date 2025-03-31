/**
* @file verification.hpp
 * @brief Functions for verifying package integrity and signatures
 *
 * Defines functions for verifying checksums and signatures of DPM packages
 * and package stage directories.
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
#include "commands.hpp"
#include "checksum.hpp"

/**
 * @brief Verifies checksums for a package file
 *
 * Checks the integrity of a package file by verifying its checksums.
 *
 * @param package_path Path to the package file
 * @return 0 on success, non-zero on failure
 */
int verify_checksums_package(const std::string& package_path);

/**
 * @brief Verifies checksums for a package stage directory
 *
 * Checks the integrity of a package stage directory by verifying its checksums.
 *
 * @param stage_dir Path to the stage directory
 * @return 0 on success, non-zero on failure
 */
int verify_checksums_stage(const std::string& stage_dir);

/**
 * @brief Verifies signatures for a package file
 *
 * Checks the signatures of a package file.
 *
 * @param package_path Path to the package file
 * @return 0 on success, non-zero on failure
 */
int verify_signature_package(const std::string& package_path);

/**
 * @brief Verifies signatures for a package stage directory
 *
 * Checks the signatures of a package stage directory.
 *
 * @param stage_dir Path to the stage directory
 * @return 0 on success, non-zero on failure
 */
int verify_signature_stage(const std::string& stage_dir);
