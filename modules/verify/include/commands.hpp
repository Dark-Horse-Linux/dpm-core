/**
 * @file commands.hpp
 * @brief Header file for the verify module command handlers
 *
 * Defines functions and enumerations for the verify module which verifies
 * the integrity and signatures of package files and stage directories.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <cstring>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <dlfcn.h>
#include <sys/stat.h>
#include <filesystem>

/**
 * @brief Handler for the checksum command
 *
 * Verifies the checksums of package files or stage directories.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_checksum(int argc, char** argv);

/**
 * @brief Handler for the checksum help command
 *
 * Displays help information for the checksum command.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success
 */
int cmd_checksum_help(int argc, char** argv);

/**
 * @brief Handler for the signature command
 *
 * Verifies the signatures of package files or stage directories.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_signature(int argc, char** argv);

/**
 * @brief Handler for the signature help command
 *
 * Displays help information for the signature command.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success
 */
int cmd_signature_help(int argc, char** argv);

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the verify module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);

/**
 * @brief Handler for the check command
 *
 * Checks if the build module can be loaded. This validates that the integration
 * between the verify module and build module is working correctly.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_check(int argc, char** argv);

/**
 * @brief Handler for the check help command
 *
 * Displays help information for the check command.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success
 */
int cmd_check_help(int argc, char** argv);

/**
 * @brief Handler for unknown commands
 *
 * Displays an error message for unrecognized commands.
 *
 * @param command The unrecognized command string
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 1 to indicate failure
 */
int cmd_unknown(const char* command, int argc, char** argv);

/**
 * @brief Helper function to check and load the build module
 *
 * Checks if the build module exists and can be loaded.
 *
 * @param module_handle Reference to a void pointer that will hold the module handle
 * @return 0 on success, non-zero on failure
 */
int check_and_load_build_module(void*& module_handle);

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