/**
 * @file error.hpp
 * @brief Error handling system for the DPM utility
 *
 * Defines the error categories, error context structure, and utility
 * functions for error handling throughout the DPM system. Provides a
 * consistent approach to error reporting and management.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * For bug reports or contributions, please contact the dhlp-contributors
 * mailing list at: https://lists.darkhorselinux.org/mailman/listinfo/dhlp-contributors
 */

#pragma once

#include <iostream>

/**
 * @enum DPMErrorCategory
 * @brief Defines error categories for the DPM system
 *
 * Enumeration of all possible error conditions that can occur during
 * DPM operations, particularly related to module loading and execution.
 */
enum class DPMErrorCategory {
    SUCCESS,               /**< Operation completed successfully */
    PATH_NOT_FOUND,        /**< The specified path does not exist */
    PATH_NOT_DIRECTORY,    /**< The specified path exists but is not a directory */
    PATH_TOO_LONG,         /**< The specified path exceeds the system's path length limit */
    PERMISSION_DENIED,     /**< Insufficient permissions to access the path */
    MODULE_NOT_FOUND,      /**< The specified module was not found in the module path */
    MODULE_NOT_LOADED,     /**< Failed to load the module (e.g., before executing it) */
    MODULE_LOAD_FAILED,    /**< Dynamic loading of the module failed */
    INVALID_MODULE,        /**< The module does not conform to the required interface */
    SYMBOL_NOT_FOUND,      /**< A required symbol was not found in the loaded module */
    SYMBOL_EXECUTION_FAILED, /**< Execution of a module function failed */
    UNDEFINED_ERROR        /**< An undefined or unexpected error occurred */
};

/**
 * @struct FlexDPMError
 * @brief Error context structure for DPM operations
 *
 * Provides context information for errors that occur during DPM operations.
 * Only the error field is required; other fields can be populated as needed
 * depending on the specific error condition.
 */
typedef struct {
    DPMErrorCategory error;     /**< The error category (required) */
    const char * module_name;   /**< Name of the module involved in the error (optional) */
    const char * module_path;   /**< Path to the modules directory (optional) */
    const char * message;       /**< Additional error message or context (optional) */
    // Add other potential fields as needed as all fields beyond error are optional
} FlexDPMError;

/**
 * @brief Creates a new FlexDPMError instance with the specified error category
 *
 * Utility function to simplify the creation of FlexDPMError structures.
 * Initializes a new error context with the given error category and
 * sets all other fields to NULL.
 *
 * @param error_category The error category to assign to the new error context
 * @return A FlexDPMError structure with the specified error category
 */
FlexDPMError make_error(DPMErrorCategory error_category);