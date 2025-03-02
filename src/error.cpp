/**
* @file error.cpp
 * @brief Implementation of error handling system for the DPM utility
 *
 * Implements the utility functions for error handling defined in error.hpp.
 * Provides functionality for creating error context structures to ensure
 * consistent error reporting throughout the DPM system.
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

#include "error.hpp"

// Simple helper function that takes only the required error category
FlexDPMError make_error(DPMErrorCategory error_category)
{
    // Create an empty error struct
    FlexDPMError error;

    // Set the error category
    error.error = error_category;

    // Initialize the other fields to NULL
    error.module_name = NULL;
    error.module_path = NULL;
    error.message = NULL;

    // let the consumer populate any other fields they want with `self.field_name = whatever`.
    return error;
}
