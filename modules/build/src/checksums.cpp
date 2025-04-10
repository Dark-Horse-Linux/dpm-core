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

extern "C" std::string get_configured_hash_algorithm()
{
    const char* algorithm = dpm_get_config("cryptography", "checksum_algorithm");
    if (algorithm && strlen(algorithm) > 0) {
        return std::string(algorithm);
    }

    // Default to SHA-256 if not specified or empty
    return "sha256";
}

extern "C" std::string get_available_algorithms()
{
    std::vector<std::string> algorithms;
    std::vector<std::string> working_algorithms;

    // Initialize OpenSSL
    OpenSSL_add_all_digests();

    // Use OBJ_NAME_do_all to get all message digest algorithms
    struct AllDigestsCallback {
        static void callback(const OBJ_NAME* obj, void* arg) {
            if (obj->type == OBJ_NAME_TYPE_MD_METH) {
                std::vector<std::string>* algs = static_cast<std::vector<std::string>*>(arg);
                algs->push_back(obj->name);
            }
        }
    };

    // Get all algorithm names
    OBJ_NAME_do_all(OBJ_NAME_TYPE_MD_METH, AllDigestsCallback::callback, &algorithms);

    // Test each algorithm with a complete hashing process
    for (const auto& algo_name : algorithms) {
        const EVP_MD* md = EVP_get_digestbyname(algo_name.c_str());
        if (!md) continue;

        // Create context
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) continue;

        // Test full hashing sequence with dummy data
        unsigned char dummy_data[] = "test";
        unsigned char out_buf[EVP_MAX_MD_SIZE];
        unsigned int out_len = 0;

        bool success = false;
        if (EVP_DigestInit_ex(ctx, md, NULL) == 1 &&
            EVP_DigestUpdate(ctx, dummy_data, sizeof(dummy_data)) == 1 &&
            EVP_DigestFinal_ex(ctx, out_buf, &out_len) == 1) {
            success = true;
            }

        EVP_MD_CTX_free(ctx);

        if (success) {
            working_algorithms.push_back(algo_name);
        }
    }

    // Format the list as a comma-separated string
    std::stringstream result;
    for (size_t i = 0; i < working_algorithms.size(); i++) {
        if (i > 0) result << ", ";
        result << working_algorithms[i];
    }

    return result.str();
}

extern "C" std::string generate_file_checksum(const std::filesystem::path& file_path)
{
    // Get configured algorithm
    std::string algorithm_name = get_configured_hash_algorithm();

    // Initialize OpenSSL
    OpenSSL_add_all_digests();

    // Get the digest
    const EVP_MD* md = EVP_get_digestbyname(algorithm_name.c_str());
    if (!md) {
        std::string available_algorithms = get_available_algorithms();
        dpm_log(LOG_FATAL, ("Hash algorithm not supported: " + algorithm_name +
                ". Available algorithms: " + available_algorithms).c_str());
        return "";
    }

    // Initialize OpenSSL EVP context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        dpm_log(LOG_ERROR, "Failed to create OpenSSL EVP context");
        return "";
    }

    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        dpm_log(LOG_ERROR, "Failed to initialize digest context");
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // Check if the file exists
    if (!std::filesystem::exists(file_path)) {
        dpm_log(LOG_ERROR, ("File does not exist: " + file_path.string()).c_str());
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

extern "C" std::string generate_string_checksum(const std::string& input_string)
{
    // Get configured algorithm
    std::string algorithm_name = get_configured_hash_algorithm();

    // Initialize OpenSSL
    OpenSSL_add_all_digests();

    // Get the digest
    const EVP_MD* md = EVP_get_digestbyname(algorithm_name.c_str());
    if (!md) {
        std::string available_algorithms = get_available_algorithms();
        dpm_log(LOG_FATAL, ("Hash algorithm not supported: " + algorithm_name +
                ". Available algorithms: " + available_algorithms).c_str());
        return "";
    }

    // Initialize OpenSSL EVP context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        dpm_log(LOG_ERROR, "Failed to create OpenSSL EVP context");
        return "";
    }

    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        dpm_log(LOG_ERROR, "Failed to initialize digest context");
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // Update digest with the string contents
    if (EVP_DigestUpdate(mdctx, input_string.c_str(), input_string.length()) != 1) {
        dpm_log(LOG_ERROR, "Failed to update digest");
        EVP_MD_CTX_free(mdctx);
        return "";
    }

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