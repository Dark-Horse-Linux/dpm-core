/**
 * @file checksum_memory.cpp
 * @brief Implementation of in-memory package checksum verification functions
 *
 * Implements functions for verifying checksums of DPM package components in memory
 * without requiring them to be extracted to disk first.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "checksum_memory.hpp"

/**
 * @brief Converts binary data to a C++ string
 *
 * Takes a buffer of binary data and its size, creates a properly
 * null-terminated string, and returns it as an std::string.
 *
 * @param data Pointer to the binary data
 * @param data_size Size of the binary data
 * @return std::string containing the data, or empty string on error
 */
std::string binary_to_string(const unsigned char* data, size_t data_size) {
    if (!data || data_size == 0) {
        return std::string();
    }

    // Create a temporary C-string with null termination
    char* temp = (char*)malloc(data_size + 1);
    if (!temp) {
        return std::string();
    }

    memcpy(temp, data, data_size);
    temp[data_size] = '\0';

    // Create std::string from the C-string
    std::string result(temp);

    // Free the temporary buffer
    free(temp);

    return result;
}

/**
 * @brief Verifies the package digest from in-memory metadata
 *
 * Calculates the package digest from in-memory CONTENTS_MANIFEST_DIGEST and
 * HOOKS_DIGEST files and compares it to the value in PACKAGE_DIGEST.
 *
 * @param package_data Pointer to the metadata component data
 * @param package_data_size Size of the metadata component data
 * @param build_module Handle to the loaded build module
 * @return 0 on successful verification, non-zero on failure
 */
int checksum_verify_package_digest_memory(
    const unsigned char* package_data,
    size_t package_data_size,
    void* build_module)
{
    // Validate input parameters
    if (!package_data || package_data_size == 0 || !build_module) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to checksum_verify_package_digest_memory");
        return 1;
    }

    dpm_log(LOG_INFO, "Verifying package digest from in-memory data...");

    // First, extract PACKAGE_DIGEST, CONTENTS_MANIFEST_DIGEST, and HOOKS_DIGEST from the metadata component
    unsigned char* package_digest_data = nullptr;
    size_t package_digest_size = 0;
    unsigned char* contents_manifest_data = nullptr;
    size_t contents_manifest_size = 0;
    unsigned char* hooks_digest_data = nullptr;
    size_t hooks_digest_size = 0;

    // Get PACKAGE_DIGEST from the metadata component
    int result = get_file_from_component(
        package_data,
        package_data_size,
        "PACKAGE_DIGEST",
        &package_digest_data,
        &package_digest_size
    );

    if (result != 0 || !package_digest_data || package_digest_size == 0) {
        dpm_log(LOG_ERROR, "Failed to extract PACKAGE_DIGEST from metadata component");
        return 1;
    }

    // Get CONTENTS_MANIFEST_DIGEST from the metadata component
    result = get_file_from_component(
        package_data,
        package_data_size,
        "CONTENTS_MANIFEST_DIGEST",
        &contents_manifest_data,
        &contents_manifest_size
    );

    if (result != 0 || !contents_manifest_data || contents_manifest_size == 0) {
        dpm_log(LOG_ERROR, "Failed to extract CONTENTS_MANIFEST_DIGEST from metadata component");
        free(package_digest_data);
        return 1;
    }

    // Get HOOKS_DIGEST from the metadata component
    result = get_file_from_component(
        package_data,
        package_data_size,
        "HOOKS_DIGEST",
        &hooks_digest_data,
        &hooks_digest_size
    );

    if (result != 0 || !hooks_digest_data || hooks_digest_size == 0) {
        dpm_log(LOG_ERROR, "Failed to extract HOOKS_DIGEST from metadata component");
        free(package_digest_data);
        free(contents_manifest_data);
        return 1;
    }

    // Convert binary data to strings using our utility function
    std::string package_digest_str = binary_to_string(package_digest_data, package_digest_size);
    std::string contents_manifest_str = binary_to_string(contents_manifest_data, contents_manifest_size);
    std::string hooks_digest_str = binary_to_string(hooks_digest_data, hooks_digest_size);

    // Check if any conversion failed
    if (package_digest_str.empty() || contents_manifest_str.empty() || hooks_digest_str.empty()) {
        dpm_log(LOG_ERROR, "Failed to convert binary data to strings");
        free(package_digest_data);
        free(contents_manifest_data);
        free(hooks_digest_data);
        return 1;
    }

    // Calculate checksums using the build module's functions through dpm_execute_symbol
    std::string contents_manifest_checksum;
    result = dpm_execute_symbol(build_module, "generate_string_checksum",
                              contents_manifest_str, &contents_manifest_checksum);

    if (result != 0 || contents_manifest_checksum.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate checksum for contents manifest");
        free(package_digest_data);
        free(contents_manifest_data);
        free(hooks_digest_data);
        return 1;
    }

    std::string hooks_digest_checksum;
    result = dpm_execute_symbol(build_module, "generate_string_checksum",
                              hooks_digest_str, &hooks_digest_checksum);

    if (result != 0 || hooks_digest_checksum.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate checksum for hooks digest");
        free(package_digest_data);
        free(contents_manifest_data);
        free(hooks_digest_data);
        return 1;
    }

    // Combine checksums and calculate package digest
    std::string combined_checksums = contents_manifest_checksum + hooks_digest_checksum;
    std::string calculated_package_digest;

    result = dpm_execute_symbol(build_module, "generate_string_checksum",
                              combined_checksums, &calculated_package_digest);

    if (result != 0 || calculated_package_digest.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate package digest");
        free(package_digest_data);
        free(contents_manifest_data);
        free(hooks_digest_data);
        return 1;
    }

    // Compare with the stored package digest
    bool match = (calculated_package_digest == package_digest_str);

    // Clean up
    free(package_digest_data);
    free(contents_manifest_data);
    free(hooks_digest_data);

    if (!match) {
        dpm_log(LOG_ERROR, ("Package digest mismatch\n  Expected: " + package_digest_str +
                           "\n  Actual:   " + calculated_package_digest).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, "Package digest verification successful");
    return 0;
}

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
    void* build_module)
{
    // Validate input parameters
    if (!contents_data || contents_data_size == 0 ||
        !metadata_data || metadata_data_size == 0 || !build_module) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to checksum_verify_contents_digest_memory");
        return 1;
    }

    dpm_log(LOG_INFO, "Verifying contents manifest digest from in-memory data...");

    // Extract CONTENTS_MANIFEST_DIGEST from the metadata component
    unsigned char* manifest_data = nullptr;
    size_t manifest_size = 0;

    int result = get_file_from_component(
        metadata_data,
        metadata_data_size,
        "CONTENTS_MANIFEST_DIGEST",
        &manifest_data,
        &manifest_size
    );

    if (result != 0 || !manifest_data || manifest_size == 0) {
        dpm_log(LOG_ERROR, "Failed to extract CONTENTS_MANIFEST_DIGEST from metadata component");
        return 1;
    }

    // Convert binary data to string
    std::string manifest_str = binary_to_string(manifest_data, manifest_size);
    if (manifest_str.empty()) {
        dpm_log(LOG_ERROR, "Failed to convert manifest data to string");
        free(manifest_data);
        return 1;
    }

    // Parse the manifest lines
    std::istringstream manifest_stream(manifest_str);
    std::string line;
    int errors = 0;
    int line_number = 0;

    // Process each line in the manifest
    while (std::getline(manifest_stream, line)) {
        line_number++;

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Parse the line into its components
        std::istringstream iss(line);
        char control_designation;
        std::string expected_checksum, permissions, ownership, file_path;

        // Extract components (C checksum permissions owner:group /path/to/file)
        if (!(iss >> control_designation >> expected_checksum >> permissions >> ownership)) {
            dpm_log(LOG_WARN, ("Malformed manifest line " + std::to_string(line_number) +
                              ": " + line).c_str());
            continue;
        }

        // Get the rest of the line as the file path
        std::getline(iss >> std::ws, file_path);

        if (file_path.empty()) {
            dpm_log(LOG_WARN, ("Missing file path in manifest line " +
                              std::to_string(line_number)).c_str());
            continue;
        }

        // Remove leading slash if present
        if (file_path[0] == '/') {
            file_path = file_path.substr(1);
        }

        // Extract the file from the contents component
        unsigned char* file_data = nullptr;
        size_t file_size = 0;

        result = get_file_from_component(
            contents_data,
            contents_data_size,
            file_path,
            &file_data,
            &file_size
        );

        if (result != 0 || !file_data) {
            dpm_log(LOG_ERROR, ("Failed to extract file from contents: " + file_path).c_str());
            errors++;
            continue;
        }

        // Calculate the checksum of the file data
        std::string calculated_checksum;
        result = dpm_execute_symbol(build_module, "generate_string_checksum",
                                  std::string(reinterpret_cast<char*>(file_data), file_size),
                                  &calculated_checksum);

        // Free the file data now that we're done with it
        free(file_data);

        if (result != 0 || calculated_checksum.empty()) {
            dpm_log(LOG_ERROR, ("Failed to calculate checksum for file: " + file_path).c_str());
            errors++;
            continue;
        }

        // Compare with the expected checksum
        if (calculated_checksum != expected_checksum) {
            dpm_log(LOG_ERROR, ("Checksum mismatch for " + file_path +
                               "\n  Expected: " + expected_checksum +
                               "\n  Actual:   " + calculated_checksum).c_str());
            errors++;
        }
    }

    // Clean up
    free(manifest_data);

    if (errors > 0) {
        dpm_log(LOG_ERROR, (std::to_string(errors) + " checksum errors found in contents manifest").c_str());
        return 1;
    }

    dpm_log(LOG_INFO, "Contents manifest checksum verification successful");
    return 0;
}

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
    void* build_module)
{
    // Validate input parameters
    if (!hooks_data || hooks_data_size == 0 ||
        !metadata_data || metadata_data_size == 0 || !build_module) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to checksum_verify_hooks_digest_memory");
        return 1;
    }

    dpm_log(LOG_INFO, "Verifying hooks digest from in-memory data...");

    // Extract HOOKS_DIGEST from the metadata component
    unsigned char* hooks_digest_data = nullptr;
    size_t hooks_digest_size = 0;

    int result = get_file_from_component(
        metadata_data,
        metadata_data_size,
        "HOOKS_DIGEST",
        &hooks_digest_data,
        &hooks_digest_size
    );

    if (result != 0 || !hooks_digest_data || hooks_digest_size == 0) {
        dpm_log(LOG_ERROR, "Failed to extract HOOKS_DIGEST from metadata component");
        return 1;
    }

    // Convert binary data to string
    std::string stored_hooks_digest = binary_to_string(hooks_digest_data, hooks_digest_size);
    if (stored_hooks_digest.empty()) {
        dpm_log(LOG_ERROR, "Failed to convert hooks digest data to string");
        free(hooks_digest_data);
        return 1;
    }

    // Trim whitespace and newlines
    stored_hooks_digest = stored_hooks_digest.substr(0, stored_hooks_digest.find_first_of("\r\n"));

    // Calculate the checksum for the hooks archive data
    std::string calculated_hooks_digest;
    result = dpm_execute_symbol(build_module, "generate_string_checksum",
                              std::string(reinterpret_cast<const char*>(hooks_data), hooks_data_size),
                              &calculated_hooks_digest);

    // Clean up
    free(hooks_digest_data);

    if (result != 0 || calculated_hooks_digest.empty()) {
        dpm_log(LOG_ERROR, "Failed to calculate hooks digest");
        return 1;
    }

    // Compare with the stored digest
    if (calculated_hooks_digest != stored_hooks_digest) {
        dpm_log(LOG_ERROR, ("Hooks digest mismatch\n  Expected: " + stored_hooks_digest +
                           "\n  Actual:   " + calculated_hooks_digest).c_str());
        return 1;
    }

    dpm_log(LOG_INFO, "Hooks digest verification successful");
    return 0;
}