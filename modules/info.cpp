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
#include <fstream>
#include <sys/utsname.h>

// Define version macros
#define MODULE_VERSION "0.1.0"
#define DPM_VERSION "0.1.0"


// Declaration of the DPM config function we want to call
extern "C" const char* dpm_get_config(const char* section, const char* key);

// Implementation of the info module
// This module provides information about the DPM system

// Version information
extern "C" const char* dpm_module_get_version(void) {
    return MODULE_VERSION;
}

// Module description
extern "C" const char* dpm_get_description(void) {
    return "DPM Info Module - Provides information about the DPM system";
}

// Command enum for switch case
enum Command {
    CMD_UNKNOWN,
    CMD_HELP,
    CMD_VERSION,
    CMD_SYSTEM,
    CMD_CONFIG
};

// Function to detect architecture using uname
std::string detect_architecture() {
    struct utsname system_info;

    if (uname(&system_info) == -1) {
        return "Unknown";
    }

    return system_info.machine;
}

// Function to detect OS
std::string detect_os() {
    struct utsname system_info;

    if (uname(&system_info) == -1) {
        return "Unknown";
    }

    std::string os = system_info.sysname;

    // For Linux, try to get distribution information
    if (os == "Linux") {
        std::ifstream os_release("/etc/os-release");
        if (os_release.is_open()) {
            std::string line;
            std::string distro_name;
            std::string distro_version;

            while (std::getline(os_release, line)) {
                if (line.find("NAME=") == 0) {
                    distro_name = line.substr(5);
                    // Remove quotes if present
                    if (distro_name.front() == '"' && distro_name.back() == '"') {
                        distro_name = distro_name.substr(1, distro_name.length() - 2);
                    }
                }
                if (line.find("VERSION_ID=") == 0) {
                    distro_version = line.substr(11);
                    // Remove quotes if present
                    if (distro_version.front() == '"' && distro_version.back() == '"') {
                        distro_version = distro_version.substr(1, distro_version.length() - 2);
                    }
                }
            }

            if (!distro_name.empty()) {
                os += " (" + distro_name;
                if (!distro_version.empty()) {
                    os += " " + distro_version;
                }
                os += ")";
            }
        }
    }

    return os;
}

// Command handler functions
int cmd_help(int argc, char** argv) {
    std::cout << "DPM Info Module - Provides information about the DPM system\n\n";
    std::cout << "Available commands:\n\n";
    std::cout << "  version    - Display DPM version information\n";
    std::cout << "  system     - Display system information\n";
    std::cout << "  config     - Display configuration information\n";
    std::cout << "  help       - Display this help message\n";
    return 0;
}

int cmd_version(int argc, char** argv) {
    std::cout << "DPM Version: " << DPM_VERSION << "\n";
    std::cout << "Build Date: " << __DATE__ << "\n";
    std::cout << "Build Time: " << __TIME__ << "\n";
    return 0;
}

int cmd_system(int argc, char** argv) {
    std::cout << "System Information:\n";
    std::cout << "  OS: " << detect_os() << "\n";
    std::cout << "  Architecture: " << detect_architecture() << "\n";
    return 0;
}

int cmd_config(int argc, char** argv) {
    const char* module_path = dpm_get_config("modules", "module_path");
    std::cout << "Configuration Information:\n";
    std::cout << "  Module Path: " << (module_path ? module_path : "Not configured") << "\n";
    return 0;
}

int cmd_unknown(const char* command, int argc, char** argv) {
    std::cerr << "Unknown command: " << (command ? command : "") << "\n";
    std::cerr << "Run 'dpm info help' for a list of available commands\n";
    return 1;
}

// Function to parse command string to enum
Command parse_command(const char* cmd_str) {
    if (cmd_str == nullptr || strlen(cmd_str) == 0) {
        return CMD_HELP;
    }

    if (strcmp(cmd_str, "help") == 0) {
        return CMD_HELP;
    }
    else if (strcmp(cmd_str, "version") == 0) {
        return CMD_VERSION;
    }
    else if (strcmp(cmd_str, "system") == 0) {
        return CMD_SYSTEM;
    }
    else if (strcmp(cmd_str, "config") == 0) {
        return CMD_CONFIG;
    }

    return CMD_UNKNOWN;
}

// Main entry point that will be called by DPM
extern "C" int dpm_module_execute(const char* command, int argc, char** argv) {
    Command cmd = parse_command(command);

    switch (cmd) {
        case CMD_VERSION:
            return cmd_version(argc, argv);

        case CMD_SYSTEM:
            return cmd_system(argc, argv);

        case CMD_CONFIG:
            return cmd_config(argc, argv);

        case CMD_HELP:
            return cmd_help(argc, argv);

        case CMD_UNKNOWN:
        default:
            return cmd_unknown(command, argc, argv);
    }
}