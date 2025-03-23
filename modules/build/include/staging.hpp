/**
* @file staging.hpp
 * @brief Functions for staging DPM packages
 *
 * Defines functions for creating and manipulating DPM package staging structures.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <pwd.h>
#include <grp.h>
#include "checksums.hpp"
#include "metadata.hpp"

/**
 * @brief Stages a DPM package
 *
 * Creates a package staging directory with the provided parameters.
 *
 * @param output_dir Directory to save the staged package
 * @param contents_dir Directory with package contents
 * @param hooks_dir Directory with package hooks (optional)
 * @param package_name Package name
 * @param package_version Package version
 * @param architecture Package architecture
 * @param os Package OS (optional)
 * @param force Force package staging even if warnings occur
 * @return 0 on success, non-zero on failure
 */
int build_package_stage(
    const std::string& output_dir,
    const std::string& contents_dir,
    const std::string& hooks_dir,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& architecture,
    const std::string& os,
    bool force
);