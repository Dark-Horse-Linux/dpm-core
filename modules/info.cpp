/**
* @file info.cpp
* @brief Implementation of the DPM info module
*
* Provides information about the DPM system through a module interface.
* This module supports commands for displaying version information,
* system details, and module help documentation.
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

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

// Implementation of the info module
// This module provides information about the DPM system

// Version information
extern "C" const char* dpm_module_get_version(void) {
    return "0.1.0";
}

// Module description
extern "C" const char* dpm_get_description(void) {
    return "DPM Info Module - Provides information about the DPM system";
}

// Main entry point that will be called by DPM
extern "C" int dpm_module_execute(const char* command, int argc, char** argv) {
    // Handle the case when no command is provided
    if (command == nullptr || strlen(command) == 0) {
        std::cout << "DPM Info Module - Provides information about the DPM system\n";
        std::cout << "Usage: dpm info <command> [args]\n";
        std::cout << "Available commands:\n";
        std::cout << "  version    - Display DPM version information\n";
        std::cout << "  system     - Display system information\n";
        std::cout << "  help       - Display this help message\n";
        return 0;
    }

    // Convert command to string for easier comparison
    std::string cmd(command);

    if (cmd == "version") {
        std::cout << "DPM Version: 0.1.0\n";
        std::cout << "Build Date: " << __DATE__ << "\n";
        std::cout << "Build Time: " << __TIME__ << "\n";
        return 0;
    }
    else if (cmd == "system") {
        std::cout << "System Information:\n";
        std::cout << "  OS: "
#ifdef _WIN32
            "Windows"
#elif __APPLE__
            "macOS"
#elif __linux__
            "Linux"
#else
            "Unknown"
#endif
            << "\n";
        std::cout << "  Architecture: "
#ifdef __x86_64__
            "x86_64"
#elif __i386__
            "x86"
#elif __arm__
            "ARM"
#elif __aarch64__
            "ARM64"
#else
            "Unknown"
#endif
            << "\n";
        return 0;
    }
    else if (cmd == "help") {
        std::cout << "DPM Info Module - Provides information about the DPM system\n";
        std::cout << "Available commands:\n";
        std::cout << "  version    - Display DPM version information\n";
        std::cout << "  system     - Display system information\n";
        std::cout << "  help       - Display this help message\n";
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << cmd << "\n";
        std::cerr << "Run 'dpm info help' for a list of available commands\n";
        return 1;
    }
}