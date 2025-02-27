/**
* @file module_interface.hpp
* @brief Defines the interface for DPM modules
*
* Establishes the required symbols and common interface that all DPM modules
* must implement to be loadable and executable by the core DPM system.
* This forms the contract between the main DPM application and its modules.
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
#include <string>
#include <vector>

/*
 *  Provides reserved symbol names we look for in modules.
 */

// Define required symbols in one place
namespace module_interface {
    // This is the single source of truth for required module symbols
    static const std::vector<std::string> required_symbols = {
        "dpm_module_execute",
        "dpm_module_get_version",
        "dpm_get_description"
    };
}

// Common interface for all DPM modules
extern "C" {
    // Module must export this symbol to be considered valid
    int dpm_module_execute(const char* command, int argc, char** argv);

    // Module version information
    const char* dpm_module_get_version(void);

    // Module description information
    const char* dpm_get_description(void);
}