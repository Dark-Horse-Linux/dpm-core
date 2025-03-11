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

    // Reset getopt
    optind = 1;
    opterr = 1; // Enable getopt error messages

    // Store original argc/argv to restore later
    int orig_argc = argc;
    char** orig_argv = argv;

    // Find the first non-option argument which should be the module name
    int module_pos = 1;
    while (module_pos < argc && argv[module_pos][0] == '-') {
        // Skip option and its value if it takes an argument
        if (argv[module_pos][1] == 'm' || argv[module_pos][1] == 'c' ||
            (strlen(argv[module_pos]) > 2 &&
             (strcmp(&argv[module_pos][1], "-module-path") == 0 ||
              strcmp(&argv[module_pos][1], "-config-dir") == 0))) {
            module_pos += 2;
        } else {
            module_pos++;
        }
    }

    // Temporarily set argc to only include global options up to the module name
    int temp_argc = module_pos;

    int opt;
    int option_index = 0;

    // Parse only the global DPM options
    while ((opt = getopt_long(temp_argc, argv, "m:c:lh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                args.module_path = optarg;
                break;
            case 'c':
                args.config_dir = optarg;
                break;
            case 'l':
                args.list_modules = true;
                break;
            case 'h':
                args.show_help = true;
                break;
            default:
                break;
        }
    }

    // Reset getopt for future calls
    optind = 1;

    // Restore original argc/argv
    argc = orig_argc;
    argv = orig_argv;

    // If we have a module name
    if (module_pos < argc) {
        args.module_name = argv[module_pos];

        // All arguments after module name go into the command string
        for (int i = module_pos + 1; i < argc; i++) {
            if (!args.command.empty()) {
                args.command += " ";
            }

            std::string arg = argv[i];
            if (arg.find(' ') != std::string::npos) {
                args.command += "\"" + arg + "\"";
            } else {
                args.command += arg;
            }
        }
    }

    return args;
}