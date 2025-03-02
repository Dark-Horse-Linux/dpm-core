/**
 * @file handlers.cpp
 * @brief Implementation of error handling functions for the DPM system
 *
 * Implements specialized handler functions for each error category in the DPM
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

#include "handlers.hpp"

// Helper function for validating required fields in a FlexDPMError
void validate_field(FlexDPMError context, const char* field_name, const void* field_value)
{
    if (!field_value) {
        std::cerr << "Error: Incomplete error context. Missing required field: " << field_name;
        std::cerr << " (Error category: " << static_cast<int>(context.error) << ")" << std::endl;

        // Hard exit when a required field is missing
        exit(1);
    }
}

// Main error handler that dispatches to specific handlers
int handle_error(FlexDPMError context) {
    if (context.error == DPMErrorCategory::SUCCESS) {
        return 0;
    }

    switch (context.error) {
        case DPMErrorCategory::PATH_NOT_FOUND:
            validate_field(context, "module_path", context.module_path);
            return handle_path_not_found(context);

        case DPMErrorCategory::PATH_NOT_DIRECTORY:
            validate_field(context, "module_path", context.module_path);
            return handle_path_not_directory(context);

        case DPMErrorCategory::PATH_TOO_LONG:
            validate_field(context, "module_path", context.module_path);
            return handle_path_too_long(context);

        case DPMErrorCategory::PERMISSION_DENIED:
            validate_field(context, "module_path", context.module_path);
            return handle_permission_denied(context);

        case DPMErrorCategory::MODULE_NOT_FOUND:
            validate_field(context, "module_name", context.module_name);
            validate_field(context, "module_path", context.module_path);
            return handle_module_not_found(context);

        case DPMErrorCategory::MODULE_NOT_LOADED:
            validate_field(context, "module_name", context.module_name);
            return handle_module_not_loaded(context);

        case DPMErrorCategory::MODULE_LOAD_FAILED:
            validate_field(context, "module_name", context.module_name);
            return handle_module_load_failed(context);

        case DPMErrorCategory::INVALID_MODULE:
            validate_field(context, "module_name", context.module_name);
            return handle_invalid_module(context);

        case DPMErrorCategory::SYMBOL_NOT_FOUND:
            validate_field(context, "module_name", context.module_name);
            return handle_symbol_not_found(context);

        case DPMErrorCategory::SYMBOL_EXECUTION_FAILED:
            validate_field(context, "module_name", context.module_name);
            return handle_symbol_execution_failed(context);

        case DPMErrorCategory::UNDEFINED_ERROR:
            return handle_undefined_error(context);

        default:
            std::cerr << "Error: Unknown error code" << std::endl;
            return 1;
    }
}

// Now the individual handlers can be simplified since required fields are guaranteed
int handle_path_not_found( FlexDPMError context ) {
    std::cerr << "Fatal error: The module directory '" << context.module_path << "' was not found. Exiting." << std::endl;
    return 1;
}

int handle_path_not_directory( FlexDPMError context ) {
    std::cerr << "Fatal error: The module path '"  << context.module_path << "' is not a directory.  Exiting." << std::endl;
    return 1;
}

int handle_path_too_long( FlexDPMError context ) {
    std::cerr << "Error: Module path is too long: '" << context.module_path << "'.  Exiting." << std::endl;
    return 1;
}

int handle_permission_denied( FlexDPMError context ) {
    std::cerr << "Error: Permission denied accessing the modules path: '" << context.module_path << "'.  Exiting." << std::endl;
    return 1;
}

int handle_module_not_found( FlexDPMError context ) {
    std::cerr << "Error: Module '"<< context.module_name << "' not found in '" << context.module_path << "'.  Exiting."  << std::endl;
    return 1;
}

int handle_module_not_loaded( FlexDPMError context ) {
    std::cerr << "Error: Attempted to execute module before loading it: " << context.module_name << std::endl;
    return 1;
}

int handle_module_load_failed( FlexDPMError context ) {
    std::cerr << "Error: Failed to load module: " << context.module_name << std::endl;
    return 1;
}

int handle_invalid_module( FlexDPMError context ) {
    std::cerr << "Error: Invalid module format: " << context.module_name << std::endl;
    return 1;
}

int handle_symbol_not_found( FlexDPMError context ) {
    std::cerr << "Error: Symbol not found in module: " << context.module_name;
    if (context.message) {
        std::cerr << " (" << context.message << ")";
    }
    std::cerr << std::endl;
    return 1;
}

int handle_symbol_execution_failed(FlexDPMError context) {
    std::cerr << "Error: Module execution failed: " << context.module_name << std::endl;
    return 1;
}

int handle_undefined_error(FlexDPMError context) {
    std::cerr << "Error: Undefined error occurred";
    if (context.module_name) {
        std::cerr << " with module: " << context.module_name;
    }
    if (context.message) {
        std::cerr << " (" << context.message << ")";
    }
    std::cerr << std::endl;
    return 1;
}
