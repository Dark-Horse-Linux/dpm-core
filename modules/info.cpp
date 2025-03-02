/**
 * @file info.cpp
 * @brief Example DPM info module implementation
 *
 * Implements a simple DPM module that provides information about the DPM system.
 * This module demonstrates how to implement the required module interface and
 * interact with the DPM core through configuration functions.
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

// Declaration of the DPM config function we want to call
extern "C" const char* dpm_get_config(const char* section, const char* key);

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
        std::cout << "  config     - Display configuration information\n";
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
        // Request config information using the direct method
        const char* module_path = dpm_get_config("modules", "module_path");

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
        std::cout << "  Module Path: " << (module_path ? module_path : "Not configured") << "\n";
        return 0;
    }
    else if (cmd == "config") {
        // Retrieve module path configuration
        const char* module_path = dpm_get_config("modules", "module_path");

        std::cout << "Configuration Information:\n";
        std::cout << "  Module Path: " << (module_path ? module_path : "Not configured") << "\n";
        return 0;
    }
    else if (cmd == "help") {
        std::cout << "DPM Info Module - Provides information about the DPM system\n";
        std::cout << "Available commands:\n";
        std::cout << "  version    - Display DPM version information\n";
        std::cout << "  system     - Display system information\n";
        std::cout << "  config     - Display configuration information\n";
        std::cout << "  help       - Display this help message\n";
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << cmd << "\n";
        std::cerr << "Run 'dpm info help' for a list of available commands\n";
        return 1;
    }
}
