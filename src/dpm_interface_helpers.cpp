/**
 * @file dpm_interface_helpers.cpp
 * @brief Implementation of helper functions for DPM command-line interface
 *
 * Implements utility functions for command-line argument parsing and provides
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

#include "dpm_interface_helpers.hpp"

CommandArgs parse_args(int argc, char* argv[])
{
    CommandArgs args;
    args.module_path = "";
    args.config_dir = "";
    args.list_modules = false;
    args.show_help = false;

    static struct option long_options[] = {
        {"module-path", required_argument, 0, 'm'},
        {"config-dir", required_argument, 0, 'c'},
        {"list-modules", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Find first non-option argument (module name)
    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            break;
        }

        // Handle option with argument
        if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--module-path") == 0) &&
            i + 1 < argc) {
            args.module_path = argv[i + 1];
            i++;  // Skip the argument value
        }
        else if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config-dir") == 0) &&
                 i + 1 < argc) {
            args.config_dir = argv[i + 1];
            i++;  // Skip the argument value
        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list-modules") == 0) {
            args.list_modules = true;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args.show_help = true;
        }
    }

    // If we found a module name
    if (i < argc) {
        // Set module name
        args.module_name = argv[i];

        // Build command string from remaining arguments
        i++;  // Move to first argument after module name
        while (i < argc) {
            if (!args.command.empty()) {
                args.command += " ";
            }

            std::string arg = argv[i];
            // Quote arguments with spaces
            if (arg.find(' ') != std::string::npos) {
                args.command += "\"" + arg + "\"";
            } else {
                args.command += arg;
            }
            i++;
        }
    }

    return args;
}