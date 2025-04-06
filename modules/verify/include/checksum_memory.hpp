/**
* @file checksum_memory.hpp
 * @brief In-memory package checksum verification functions
 *
 * Defines functions for verifying checksums of DPM package components in memory
 * without requiring them to be extracted to disk first.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include "package_operations.hpp"
#include <filesystem>
#include <dlfcn.h>

/**
 * @brief Verifies the package digest from in-memory metadata
 *
 * Calculates the package digest from in-memory CONTENTS_MANIFEST_DIGEST and
 * HOOKS_DIGEST files and compares it to the value in PACKAGE_DIGEST.
 *
 * @param data Pointer to the metadata file data
 * @param data_size Size of the metadata file data
 * @param build_module Handle to the loaded build module
 * @return 0 on successful verification, non-zero on failure
 */
int checksum_verify_package_digest_memory(
    const unsigned char* data,
    size_t data_size,
    void* build_module);

/**
 * @brief Verifies the contents manifest digest from in-memory data
 *
 * Compares checksums in the contents manifest with actual file checksums
 * using in-memory data rather than extracting files to disk.
 *
 * @param contents_data Pointer to the contents component data
 * @param contents_data_size Size of the contents component data
 * @param metadata_data Pointer to the metadata component data
 * @param metadata_data_size Size of the metadata component data
 * @param build_module Handle to the loaded build module
 * @return 0 on successful verification, non-zero on failure
 */
int checksum_verify_contents_digest_memory(
    const unsigned char* contents_data,
    size_t contents_data_size,
    const unsigned char* metadata_data,
    size_t metadata_data_size,
    void* build_module);

/**
 * @brief Verifies the hooks digest from in-memory data
 *
 * Calculates the digest of the hooks archive and compares it with the
 * value stored in HOOKS_DIGEST metadata file.
 *
 * @param hooks_data Pointer to the hooks component data
 * @param hooks_data_size Size of the hooks component data
 * @param metadata_data Pointer to the metadata component data
 * @param metadata_data_size Size of the metadata component data
 * @param build_module Handle to the loaded build module
 * @return 0 on successful verification, non-zero on failure
 */
int checksum_verify_hooks_digest_memory(
    const unsigned char* hooks_data,
    size_t hooks_data_size,
    const unsigned char* metadata_data,
    size_t metadata_data_size,
    void* build_module);