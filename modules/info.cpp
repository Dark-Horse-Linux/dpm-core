// info.cpp
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

// Declaration of the DPM core function we want to call
extern "C" int dpm_core_callback(const char* action, const char* data);

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

        dpm_core_callback("log", "No command specified, displayed help");
        return 0;
    }

    // Convert command to string for easier comparison
    std::string cmd(command);

    if (cmd == "version") {
        dpm_core_callback("log", "Executing 'version' command");

        std::cout << "DPM Version: 0.1.0\n";
        std::cout << "Build Date: " << __DATE__ << "\n";
        std::cout << "Build Time: " << __TIME__ << "\n";
        return 0;
    }
    else if (cmd == "system") {
        dpm_core_callback("log", "Executing 'system' command");

        // Request config information
        dpm_core_callback("get_config", "system.arch");

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
        dpm_core_callback("log", "Executing 'help' command");

        std::cout << "DPM Info Module - Provides information about the DPM system\n";
        std::cout << "Available commands:\n";
        std::cout << "  version    - Display DPM version information\n";
        std::cout << "  system     - Display system information\n";
        std::cout << "  help       - Display this help message\n";
        return 0;
    }
    else {
        std::string error_msg = "Unknown command: " + cmd;
        dpm_core_callback("log", error_msg.c_str());

        std::cerr << "Unknown command: " << cmd << "\n";
        std::cerr << "Run 'dpm info help' for a list of available commands\n";
        return 1;
    }
}