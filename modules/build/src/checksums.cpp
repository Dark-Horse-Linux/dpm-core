/**
 * @file checksums.cpp
 * @brief Implementation of cryptographic checksum functions
 *
 * Implements functionality for generating checksums of files using
 * configurable cryptographic hash algorithms via OpenSSL.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "checksums.hpp"
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <dpmdk/include/CommonModuleAPI.hpp>

std::string get_configured_hash_algorithm()
{
    const char* algorithm = dpm_get_config("cryptography", "checksum_algorithm");
    if (algorithm) {
        return std::string(algorithm);
    }
    return "sha256"; // Default to SHA-256
}

std::string get_available_algorithms()
{
    std::stringstream algorithms;
    bool first = true;

    // Only list algorithms that are actually usable for file checksums
    const char* usable_digests[] = {
        "md5", "sha1", "sha224", "sha256", "sha384", "sha512"
    };

    for (const char* digest : usable_digests) {
        // Verify the algorithm is actually available in this OpenSSL build
        if (EVP_get_digestbyname(digest) != nullptr) {
            if (!first) {
                algorithms << ", ";
            }
            algorithms << digest;
            first = false;
        }
    }

    return algorithms.str();
}

std::string generate_file_checksum(const std::filesystem::path& file_path)
{
    // Get configured algorithm
    std::string algorithm_name = get_configured_hash_algorithm();
    dpm_log(LOG_DEBUG, ("Using hash algorithm: " + algorithm_name).c_str());

    // Initialize OpenSSL EVP context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        dpm_log(LOG_ERROR, "Failed to create OpenSSL EVP context");
        return "";
    }

    // Let OpenSSL look up the algorithm by name
    const EVP_MD* md = EVP_get_digestbyname(algorithm_name.c_str());
    if (!md) {
        std::string available_algorithms = get_available_algorithms();
        std::string error_msg = "Hash algorithm not supported by OpenSSL: " + algorithm_name +
                     ". Please check your cryptography.checksum_algorithm configuration.\n" +
                     "Available algorithms: " + available_algorithms;
        dpm_log(LOG_FATAL, error_msg.c_str());
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        dpm_log(LOG_ERROR, "Failed to initialize digest context");
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // Open the file for reading in binary mode
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        dpm_log(LOG_ERROR, ("Failed to open file for checksum: " + file_path.string()).c_str());
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // Buffer for reading file chunks
    const size_t buffer_size = 8192;  // 8KB chunks
    unsigned char buffer[buffer_size];

    // Read and update digest
    while (file) {
        file.read(reinterpret_cast<char*>(buffer), buffer_size);
        size_t bytes_read = file.gcount();

        if (bytes_read > 0) {
            if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
                dpm_log(LOG_ERROR, "Failed to update digest");
                EVP_MD_CTX_free(mdctx);
                file.close();
                return "";
            }
        }
    }

    file.close();

    // Finalize the digest
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        dpm_log(LOG_ERROR, "Failed to finalize digest");
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    EVP_MD_CTX_free(mdctx);

    // Convert binary hash to hexadecimal string
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}