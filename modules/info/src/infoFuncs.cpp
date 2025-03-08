/**
 * @file infoFuncs.cpp
 * @brief Implementation of the info module support functions
 *
 * Implements functions for the info module that provide information about
 * the DPM system, including version, system details, and configuration.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "infoFuncs.hpp"


/**
 * Function to detect architecture using uname
 */
std::string detect_architecture() {
    struct utsname system_info;

    if (uname(&system_info) == -1) {
        dpm_log(LOG_ERROR, "Failed to detect system architecture");
        return "Unknown";
    }

    return system_info.machine;
}

/**
 * Function to detect OS details
 */
std::string detect_os() {
    struct utsname system_info;

    if (uname(&system_info) == -1) {
        dpm_log(LOG_ERROR, "Failed to detect operating system");
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

/**
 * Command handler for help command
 */
int cmd_help(int argc, char** argv) {
    dpm_log(LOG_INFO, "DPM Info Module - Provides information about the DPM system");
    dpm_log(LOG_INFO, "Available commands:");
    dpm_log(LOG_INFO, "  version    - Display DPM version information");
    dpm_log(LOG_INFO, "  system     - Display system information");
    dpm_log(LOG_INFO, "  config     - Display configuration information");
    dpm_log(LOG_INFO, "  help       - Display this help message");
    return 0;
}

/**
 * Command handler for version command
 */
int cmd_version(int argc, char** argv) {
    std::string version_msg = "DPM Version: ";
    version_msg += DPM_VERSION;
    dpm_log(LOG_INFO, version_msg.c_str());

    std::string date_msg = "Build Date: ";
    date_msg += __DATE__;
    dpm_log(LOG_INFO, date_msg.c_str());

    std::string time_msg = "Build Time: ";
    time_msg += __TIME__;
    dpm_log(LOG_INFO, time_msg.c_str());

    return 0;
}

/**
 * Command handler for system command
 */
int cmd_system(int argc, char** argv) {
    dpm_log(LOG_INFO, "System Information:");

    std::string os_msg = "  OS: ";
    os_msg += detect_os();
    dpm_log(LOG_INFO, os_msg.c_str());

    std::string arch_msg = "  Architecture: ";
    arch_msg += detect_architecture();
    dpm_log(LOG_INFO, arch_msg.c_str());

    return 0;
}

/**
 * Command handler for config command
 */
int cmd_config(int argc, char** argv) {
    const char* module_path = dpm_get_config("modules", "module_path");

    dpm_log(LOG_INFO, "Configuration Information:");

    std::string path_msg = "  Module Path: ";
    path_msg += (module_path ? module_path : "Not configured");
    dpm_log(LOG_INFO, path_msg.c_str());

    return 0;
}

/**
 * Command handler for unknown commands
 */
int cmd_unknown(const char* command, int argc, char** argv) {
    std::string msg = "Unknown command: ";
    msg += (command ? command : "");
    dpm_log(LOG_WARN, msg.c_str());
    dpm_log(LOG_WARN, "Run 'dpm info help' for a list of available commands");
    return 1;
}

/**
 * Function to parse command string to enum
 */
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
