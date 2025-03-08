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

    static struct option long_options[] = {
        {"module-path", required_argument, 0, 'm'},
        {"config-dir", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:c:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                args.module_path = optarg;
                break;
            case 'c':
                args.config_dir = optarg;
                break;
            case 'h':
                std::cout << "Usage: dpm [options] [module-name] [module args...] [module-command] [command-args]\n\n"
                          << "Options:\n\n"
                          << "  -m, --module-path PATH   Path to DPM modules (overrides modules.modules_path in config)\n"
                          << "  -c, --config-dir PATH    Path to DPM configuration directory\n"
                          << "  -h, --help               Show this help message\n\n";
                exit(0);
            case '?':
                exit(1);
        }
    }

    if (optind < argc) {
        args.module_name = argv[optind++];

        for (int i = optind; i < argc; i++) {
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
