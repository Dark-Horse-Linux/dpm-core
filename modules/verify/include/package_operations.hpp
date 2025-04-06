/**
 * @file package_operations.hpp
 * @brief Functions for operating on DPM packages
 *
 * Defines functions for extracting and verifying components from DPM packages.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include "commands.hpp"
#include <filesystem>

/**
 * @brief Extracts a component from a package file
 *
 * Loads a component (metadata, contents, hooks, signatures) from a package file
 * by calling into the build module's get_file_from_package_file function.
 *
 * @param package_path Path to the package file
 * @param component_name Name of the component to extract (metadata, contents, hooks, signatures)
 * @param data Pointer to a pointer that will be populated with the component data
 * @param data_size Pointer to a size_t that will be populated with the size of the component data
 * @return 0 on success, non-zero on failure
 */
int get_component_from_package(const std::string& package_path,
                              const std::string& component_name,
                              unsigned char** data,
                              size_t* data_size);

/**
 * @brief Extracts a file from a component archive
 *
 * Extracts a specific file from a component archive that has already been loaded into memory.
 * Uses the build module's get_file_from_memory_loaded_archive function.
 *
 * @param component_data Pointer to the component archive data in memory
 * @param component_size Size of the component archive in memory
 * @param filename Name of the file to extract from the component
 * @param data Pointer to a pointer that will be populated with the file data
 * @param data_size Pointer to a size_t that will be populated with the size of the file data
 * @return 0 on success, non-zero on failure
 */
int get_file_from_component(const unsigned char* component_data,
                           size_t component_size,
                           const std::string& filename,
                           unsigned char** data,
                           size_t* data_size);
