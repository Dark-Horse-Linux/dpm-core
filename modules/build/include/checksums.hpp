/**
* @file checksums.hpp
 * @brief Functions for generating cryptographic checksums
 *
 * Provides functionality for generating checksums of files using
 * configurable cryptographic hash algorithms via OpenSSL.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <dpmdk/include/CommonModuleAPI.hpp>
#include <vector>
#include <cstring>
#include <algorithm>

/**
 * @brief Gets the configured hash algorithm or defaults to SHA-256
 *
 * Retrieves the hash algorithm configured in the cryptography section
 * or defaults to SHA-256 if not specified.
 *
 * @return String containing the name of the hash algorithm to use
 */
extern "C" std::string get_configured_hash_algorithm();

/**
 * @brief Gets a list of available digest algorithms from OpenSSL
 *
 * Retrieves a list of supported digest algorithms from OpenSSL's
 * internal algorithms list.
 *
 * @return String containing comma-separated list of available algorithms
 */
extern "C" std::string get_available_algorithms();

/**
 * @brief Generates a file checksum using the configured hashing algorithm
 *
 * Uses OpenSSL to calculate a cryptographic hash of a file's contents
 * based on the algorithm specified in the configuration.
 * This method reads the file in chunks to handle large files efficiently.
 *
 * @param file_path Path to the file to be hashed
 * @return String containing the hexadecimal representation of the checksum, or empty string on error
 */
extern "C" std::string generate_file_checksum(const std::filesystem::path& file_path);

/**
 * @brief Generates a checksum of a string using the configured hashing algorithm
 *
 * Uses OpenSSL to calculate a cryptographic hash of a string's contents
 * based on the algorithm specified in the configuration.
 *
 * @param input_string The string to be hashed
 * @return String containing the hexadecimal representation of the checksum, or empty string on error
 */
extern "C" std::string generate_string_checksum(const std::string& input_string);