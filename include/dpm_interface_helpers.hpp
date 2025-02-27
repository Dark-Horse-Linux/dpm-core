/**
* @file dpm_interface_helpers.hpp
 * @brief Helper functions for DPM command-line interface
 *
 * Provides utility functions for command-line argument parsing and
 * data structures for representing command arguments in a structured format.
 * These helpers are used by the main DPM interface to process user input.
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
#include <iostream>
#include <getopt.h>

// data structure for supplied arguments
struct CommandArgs {
    std::string module_path;
    std::string module_name;
    std::string command;  // All arguments combined into a single command string
};

// parse dpm cli arguments into a serialized structure
CommandArgs parse_args( int argc, char * argv[] );
