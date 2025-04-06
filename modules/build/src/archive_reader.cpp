/**
 * @file archive_reader.cpp
 * @brief Implementation of in-memory archive reading and verification
 */

#include "archive_reader.hpp"
#include "checksums.hpp"
#include <vector>
#include <map>
#include <archive.h>
#include <archive_entry.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>


/**
 * Extracts a specific file from a package file (gzipped tarball)
 *
 * @param package_file_path Path to the package file (.dpm)
 * @param file_path_in_archive Path of the file to extract within the archive
 * @param data Pointer to buffer pointer - will be allocated by function
 * @param data_size Pointer to size variable that will receive file size
 * @return true on success, false on failure
 */
extern "C" bool get_file_from_package_file(const char* package_file_path, const char* file_path_in_archive, unsigned char** data, size_t* data_size)
{
    if (!package_file_path || !file_path_in_archive || !data || !data_size) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to get_file_from_package_file");
        return false;
    }

    // Initialize output parameters
    *data = NULL;
    *data_size = 0;

    // Create a new archive for reading
    struct archive* a = archive_read_new();
    if (!a) {
        dpm_log(LOG_ERROR, "Failed to create archive object");
        return false;
    }

    // Enable support for gzipped tarballs
    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    // Open the package file - using 0 for block size lets libarchive choose the optimal size
    int r = archive_read_open_filename(a, package_file_path, 0);
    if (r != ARCHIVE_OK) {
        dpm_log(LOG_ERROR, ("Failed to open package file: " + std::string(package_file_path) +
                           " - " + std::string(archive_error_string(a))).c_str());
        archive_read_free(a);
        return false;
    }

    // Iterate through archive entries
    bool found = false;
    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* current_path = archive_entry_pathname(entry);

        // Check if this is the file we're looking for
        if (strcmp(current_path, file_path_in_archive) == 0) {
            // Get the file size
            size_t file_size = archive_entry_size(entry);
            *data_size = file_size;

            // Allocate buffer of appropriate size
            *data = (unsigned char*)malloc(file_size);
            if (!*data) {
                dpm_log(LOG_ERROR, "Failed to allocate memory for file contents");
                archive_read_free(a);
                return false;
            }

            // Read the file content into the buffer
            ssize_t bytes_read = archive_read_data(a, *data, file_size);
            if (bytes_read < 0 || (size_t)bytes_read != file_size) {
                dpm_log(LOG_ERROR, ("Failed to read file data: " +
                                  std::string(archive_error_string(a))).c_str());
                free(*data);
                *data = NULL;
                *data_size = 0;
                archive_read_free(a);
                return false;
            }

            found = true;
            break;
        }

        // Skip to next entry
        archive_read_data_skip(a);
    }

    // Clean up
    archive_read_free(a);

    if (!found) {
        dpm_log(LOG_ERROR, ("File not found in package: " +
                          std::string(file_path_in_archive)).c_str());
        return false;
    }

    return true;
}

/**
 * Extracts a specific file from an in-memory archive (gzipped tarball)
 *
 * @param archive_data Pointer to the archive data in memory
 * @param archive_data_size Size of the archive data in memory
 * @param file_path_in_archive Path of the file to extract within the archive
 * @param result_data Pointer to buffer pointer - will be allocated by function
 * @param result_data_size Pointer to size variable that will receive file size
 * @return true on success, false on failure
 */
extern "C" bool get_file_from_memory_loaded_archive(const unsigned char* archive_data, const size_t archive_data_size,
                                         const char* file_path_in_archive,
                                         unsigned char** result_data, size_t* result_data_size)
{
    if (!archive_data || archive_data_size == 0 || !file_path_in_archive ||
        !result_data || !result_data_size) {
        dpm_log(LOG_ERROR, "Invalid parameters passed to get_file_from_memory_loaded_archive");
        return false;
    }

    // Initialize output parameters
    *result_data = NULL;
    *result_data_size = 0;

    // Create a new archive for reading
    struct archive* a = archive_read_new();
    if (!a) {
        dpm_log(LOG_ERROR, "Failed to create archive object");
        return false;
    }

    // Enable support for gzipped tarballs
    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    // Open the archive from memory
    int r = archive_read_open_memory(a, (void*)archive_data, archive_data_size);
    if (r != ARCHIVE_OK) {
        dpm_log(LOG_ERROR, ("Failed to open archive from memory: " +
                          std::string(archive_error_string(a))).c_str());
        archive_read_free(a);
        return false;
    }

    // Iterate through archive entries
    bool found = false;
    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* current_path = archive_entry_pathname(entry);

        // Check if this is the file we're looking for
        if (strcmp(current_path, file_path_in_archive) == 0) {
            // Get the file size
            size_t file_size = archive_entry_size(entry);
            *result_data_size = file_size;

            // Allocate buffer of appropriate size
            *result_data = (unsigned char*)malloc(file_size);
            if (!*result_data) {
                dpm_log(LOG_ERROR, "Failed to allocate memory for file contents");
                archive_read_free(a);
                return false;
            }

            // Read the file content into the buffer
            ssize_t bytes_read = archive_read_data(a, *result_data, file_size);
            if (bytes_read < 0 || (size_t)bytes_read != file_size) {
                dpm_log(LOG_ERROR, ("Failed to read file data from memory archive: " +
                                  std::string(archive_error_string(a))).c_str());
                free(*result_data);
                *result_data = NULL;
                *result_data_size = 0;
                archive_read_free(a);
                return false;
            }

            found = true;
            break;
        }

        // Skip to next entry
        archive_read_data_skip(a);
    }

    // Clean up
    archive_read_free(a);

    if (!found) {
        dpm_log(LOG_ERROR, ("File not found in memory archive: " +
                          std::string(file_path_in_archive)).c_str());
        return false;
    }

    return true;
}

