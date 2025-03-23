/**
* @file signing.hpp
 * @brief Functions for signing DPM packages
 *
 * Defines functions for signing DPM package stages and package files.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <gpgme.h>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include "helpers.hpp"
#include "sealing.hpp"

/**
 * @brief Signs a package stage directory
 *
 * Creates detached GPG signatures for the contents, hooks, and metadata
 * components of a package stage directory.
 *
 * @param stage_dir Path to the package stage directory
 * @param key_id GPG key ID or email to use for signing
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int sign_stage_directory(const std::string& stage_dir, const std::string& key_id, bool force);

/**
 * @brief Signs a package file
 *
 * Extracts a package file, signs its components, and creates a new signed package.
 *
 * @param package_path Path to the package file
 * @param key_id GPG key ID or email to use for signing
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int sign_package_file(const std::string& package_path, const std::string& key_id, bool force);