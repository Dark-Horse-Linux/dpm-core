/**
 * @file sealing.hpp
 * @brief Functions for sealing and unsealing DPM packages
 *
 * Defines functions for compressing and packaging DPM package stage directories
 * into the final distributable format, as well as extracting them back to the
 * stage format.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <zlib.h>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include "helpers.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <unistd.h>
#include <metadata.hpp>

/**
 * @brief First phase of sealing a package stage directory
 *
 * Replaces contents, metadata, hooks, and signatures directories with
 * gzipped tarballs, creating the intermediate package format.
 *
 * @param stage_dir Path to the package stage directory
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int seal_stage_components( const std::string& stage_dir, bool force );

/**
 * @brief Second phase of sealing to finalize a package
 *
 * Ensures all components are already sealed (compressed), then
 * creates a final package by compressing the entire stage directory.
 *
 * @param stage_dir Path to the package stage directory
 * @param output_dir Path to directory where final package should be placed (optional)
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int seal_final_package(const std::string &stage_dir, const std::string &output_dir, bool force);

/**
 * @brief Unseals a package file back to stage format
 *
 * Extracts a sealed package file back to its original stage directory structure
 * by expanding the gzipped tarballs.
 *
 * @param package_path Path to the sealed package file
 * @param output_dir Path to extract the package stage to
 * @param force Whether to force the operation even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int unseal_package(const std::string& package_path, const std::string& output_dir, bool force);


/**
 * @brief Unseals component files in a stage directory
 *
 * Finds compressed component files in a stage directory and uncompresses them
 * in place to their proper directory form.
 *
 * @param stage_dir Path to the stage directory containing components
 * @return 0 on success, non-zero on failure
 */
int unseal_stage_components(const std::filesystem::path& stage_dir);