/**
* @file archive_reader.hpp
 * @brief Functions for in-memory archive reading and verification
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <archive.h>
#include <archive_entry.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include "checksums.hpp"

extern "C" {
    /**
     * Extracts a specific file from a package file (gzipped tarball)
     *
     * @param package_file_path Path to the package file (.dpm)
     * @param file_path_in_archive Path of the file to extract within the archive
     * @param data Pointer to buffer pointer - will be allocated by function
     * @param data_size Pointer to size variable that will receive file size
     * @return true on success, false on failure
     */
    bool get_file_from_package_file(const char* package_file_path, const char* file_path_in_archive, unsigned char** data, size_t* data_size);

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
    bool get_file_from_memory_loaded_archive(const unsigned char* archive_data, const size_t archive_data_size,
                                            const char* file_path_in_archive,
                                            unsigned char** result_data, size_t* result_data_size);
}