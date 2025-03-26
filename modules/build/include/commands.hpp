#pragma once

#include "cli_parsers.hpp"
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <filesystem>
#include "staging.hpp"
#include "signing.hpp"
#include "sealing.hpp"  // Added this include
#include <map>
#include <sstream>

/**
 * @brief Handler for the stage command
 *
 * Processes arguments and stages a DPM package.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_stage(int argc, char** argv);

/**
 * @brief Handler for the manifest command
 *
 * Generates or refreshes package manifest.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_metadata(int argc, char** argv);

/**
 * @brief Handler for the sign command
 *
 * Signs a DPM package or package stage directory using GPG.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_sign(int argc, char** argv);

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the build module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_help(int argc, char** argv);

/**
 * @brief Handler for the help command
 *
 * Displays information about available commands in the build module.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_stage_help(int argc, char** argv);

/**
 * @brief Handler for the sign help command
 *
 * Displays information about sign command options.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_sign_help(int argc, char** argv);

/**
 * @brief Handler for the manifest help command
 *
 * Displays information about manifest command options.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_metadata_help(int argc, char** argv);

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
 * @brief Handler for the seal command
 *
 * Seals a DPM package stage directory by replacing contents, metadata,
 * hooks, and signatures directories with gzipped tarballs.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_seal(int argc, char** argv);

/**
 * @brief Handler for the seal help command
 *
 * Displays information about seal command options.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_seal_help(int argc, char** argv);

/**
 * @brief Handler for the unseal command
 *
 * Unseals a DPM package file by extracting and expanding the gzipped
 * tarballs back into a package stage directory structure.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_unseal(int argc, char** argv);

/**
 * @brief Handler for the unseal help command
 *
 * Displays information about unseal command options.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return 0 on success, non-zero on failure
 */
int cmd_unseal_help(int argc, char** argv);

