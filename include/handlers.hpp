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

// fatal error routing method
int handle_error(FlexDPMError context);

// declare a required field
void validate_field(FlexDPMError context, const char* field_name, const void* field_value);

// Individual error handler prototypes
int handle_path_not_found(FlexDPMError context);
int handle_path_not_directory(FlexDPMError context);
int handle_path_too_long(FlexDPMError context);
int handle_permission_denied(FlexDPMError context);
int handle_module_not_found(FlexDPMError context);
int handle_module_not_loaded(FlexDPMError context);
int handle_module_load_failed(FlexDPMError context);
int handle_invalid_module(FlexDPMError context);
int handle_symbol_not_found(FlexDPMError context);
int handle_symbol_execution_failed(FlexDPMError context);
int handle_undefined_error(FlexDPMError context);
