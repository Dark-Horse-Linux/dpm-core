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

// global errors for the core DPM routing/execution component
enum class DPMErrorCategory {
    SUCCESS,
    PATH_NOT_FOUND,
    PATH_NOT_DIRECTORY,
    PATH_TOO_LONG,
    PERMISSION_DENIED,
    MODULE_NOT_FOUND,
    MODULE_NOT_LOADED,
    MODULE_LOAD_FAILED,
    INVALID_MODULE,
    SYMBOL_NOT_FOUND,
    SYMBOL_EXECUTION_FAILED,
    UNDEFINED_ERROR
};

// A generic context object that can hold any error-specific data
// only DPMErrorCategory is required, all other fields are optional
typedef struct {
    DPMErrorCategory error;
    const char * module_name;
    const char * module_path;
    const char * message;
    // Add other potential fields as needed as all fields beyond error are optional
} FlexDPMError;

// shorthand for creating a FlexDPMError instance
FlexDPMError make_error( DPMErrorCategory error_category );

