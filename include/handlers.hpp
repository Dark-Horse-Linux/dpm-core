/**
 * @file handlers.hpp
 * @brief Error handling functions for the DPM system
 *
 * Defines specialized handler functions for each error category in the DPM
 * error system. These handlers translate error codes into user-friendly
 * messages and provide appropriate exit behavior for different error conditions.
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
#include "error.hpp"

/**
 * @brief Main error handler that dispatches to specific handlers
 *
 * Routes an error to the appropriate specialized handler based on the
 * error category in the context. Validates required fields before calling
 * the specialized handler.
 *
 * @param context Error context containing the error category and related information
 * @return Exit code appropriate for the error condition (0 for success, non-zero for errors)
 */
int handle_error(FlexDPMError context);

/**
 * @brief Validates that a required field is present in the error context
 *
 * Checks if a field that is required for a specific error category is
 * present in the error context. If the field is missing, outputs an
 * error message and exits the program.
 *
 * @param context Error context to validate
 * @param field_name Name of the field being validated (for error reporting)
 * @param field_value Pointer to the field value
 */
void validate_field(FlexDPMError context, const char* field_name, const void* field_value);

/**
 * @brief Handler for PATH_NOT_FOUND errors
 *
 * Reports that a specified path does not exist.
 *
 * @param context Error context with module_path field populated
 * @return Exit code for the error condition
 */
int handle_path_not_found(FlexDPMError context);

/**
 * @brief Handler for PATH_NOT_DIRECTORY errors
 *
 * Reports that a specified path exists but is not a directory.
 *
 * @param context Error context with module_path field populated
 * @return Exit code for the error condition
 */
int handle_path_not_directory(FlexDPMError context);

/**
 * @brief Handler for PATH_TOO_LONG errors
 *
 * Reports that a specified path exceeds the system's path length limit.
 *
 * @param context Error context with module_path field populated
 * @return Exit code for the error condition
 */
int handle_path_too_long(FlexDPMError context);

/**
 * @brief Handler for PERMISSION_DENIED errors
 *
 * Reports insufficient permissions to access a path.
 *
 * @param context Error context with module_path field populated
 * @return Exit code for the error condition
 */
int handle_permission_denied(FlexDPMError context);

/**
 * @brief Handler for MODULE_NOT_FOUND errors
 *
 * Reports that a specified module was not found in the module path.
 *
 * @param context Error context with module_name and module_path fields populated
 * @return Exit code for the error condition
 */
int handle_module_not_found(FlexDPMError context);

/**
 * @brief Handler for MODULE_NOT_LOADED errors
 *
 * Reports that a module could not be loaded.
 *
 * @param context Error context with module_name field populated
 * @return Exit code for the error condition
 */
int handle_module_not_loaded(FlexDPMError context);

/**
 * @brief Handler for MODULE_LOAD_FAILED errors
 *
 * Reports that dynamic loading of a module failed.
 *
 * @param context Error context with module_name field populated
 * @return Exit code for the error condition
 */
int handle_module_load_failed(FlexDPMError context);

/**
 * @brief Handler for INVALID_MODULE errors
 *
 * Reports that a module does not conform to the required interface.
 *
 * @param context Error context with module_name field populated
 * @return Exit code for the error condition
 */
int handle_invalid_module(FlexDPMError context);

/**
 * @brief Handler for SYMBOL_NOT_FOUND errors
 *
 * Reports that a required symbol was not found in a loaded module.
 *
 * @param context Error context with module_name field populated
 * @return Exit code for the error condition
 */
int handle_symbol_not_found(FlexDPMError context);

/**
 * @brief Handler for SYMBOL_EXECUTION_FAILED errors
 *
 * Reports that execution of a module function failed.
 *
 * @param context Error context with module_name field populated
 * @return Exit code for the error condition
 */
int handle_symbol_execution_failed(FlexDPMError context);

/**
 * @brief Handler for UNDEFINED_ERROR errors
 *
 * Reports an undefined or unexpected error.
 *
 * @param context Error context with optional module_name and message fields
 * @return Exit code for the error condition
 */
int handle_undefined_error(FlexDPMError context);