/**
 * @file ConfigManager.hpp
 * @brief Configuration management system for the DPM utility
 *
 * Defines the ConfigManager class which is responsible for loading, parsing,
 * and providing access to configuration values from INI-style files in the
 * /etc/dpm/conf.d/ directory. Supports the configuration needs of both the
 * DPM core and its modules.
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
#include <map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <string.h>
#include <dirent.h>

#include "dpm_interface_helpers.hpp"

/**
 * @class ConfigManager
 * @brief Manages and provides access to configuration settings
 *
 * This class handles loading, parsing, and providing access to configuration
 * values from INI-style files. It supports sections, key-value pairs, and
 * provides type-conversion methods for different value types.
 */
class ConfigManager {
    public:
        /**
         * @brief Constructor
         *
         * Initializes a new ConfigManager instance with the default
         * configuration directory.
         */
        ConfigManager();

        /**
         * @brief Sets the configuration directory path
         *
         * @param config_dir The directory path where configuration files are located
         */
        void setConfigDir(const std::string& config_dir);

        /**
         * @brief Gets the current configuration directory path
         *
         * @return The current configuration directory path
         */
        std::string getConfigDir() const;

        /**
         * @brief Loads all configuration files from the config directory
         *
         * Scans the configuration directory for .conf files, parses them,
         * and populates the internal configuration data structure.
         *
         * @return true if at least one configuration file was loaded successfully,
         *         false otherwise
         */
        bool loadConfigurations();

        /**
         * @brief Gets a configuration value as a C-style string
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @return The configuration value as a C-style string, or NULL if not found
         */
        const char* getConfigValue(const char* section, const char* key) const;

        /**
         * @brief Gets a configuration value as a C++ string with default
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @param defaultValue The default value to return if the key is not found
         * @return The configuration value as a string, or defaultValue if not found
         */
        std::string getConfigString(const char* section, const char* key, const std::string& defaultValue = "") const;

        /**
         * @brief Gets a configuration value as an integer with default
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @param defaultValue The default value to return if the key is not found or conversion fails
         * @return The configuration value as an integer, or defaultValue if not found or conversion fails
         */
        int getConfigInt(const char* section, const char* key, int defaultValue = 0) const;

        /**
         * @brief Gets a configuration value as a double with default
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @param defaultValue The default value to return if the key is not found or conversion fails
         * @return The configuration value as a double, or defaultValue if not found or conversion fails
         */
        double getConfigDouble(const char* section, const char* key, double defaultValue = 0.0) const;

        /**
         * @brief Gets a configuration value as a boolean with default
         *
         * Recognizes "true", "yes", "1", "on", "enabled" as true values and
         * "false", "no", "0", "off", "disabled" as false values (case-insensitive).
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @param defaultValue The default value to return if the key is not found or conversion fails
         * @return The configuration value as a boolean, or defaultValue if not found or conversion fails
         */
        bool getConfigBool(const char* section, const char* key, bool defaultValue = false) const;

        /**
         * @brief Checks if the configuration directory exists
         *
         * @return true if the configuration directory exists, false otherwise
         */
        bool configDirExists() const;

        /**
         * @brief Checks if a configuration key exists
         *
         * @param section The section name (uses DEFAULT_SECTION if NULL)
         * @param key The configuration key
         * @return true if the key exists in the specified section or DEFAULT_SECTION, false otherwise
         */
        bool hasConfigKey(const char* section, const char* key) const;

        // getter for _module_path
        void setModulePath(const char * module_path);

        // setter for _module_path
        const char * getModulePath() const;

    private:
        /**
         * @brief Default section name to use when none is specified
         */
        static constexpr const char* DEFAULT_SECTION = "MAIN";

        /**
         * @brief Parses a single configuration file
         *
         * @param config_file Path to the configuration file to parse
         * @return true if the file was parsed successfully, false otherwise
         */
        bool parseConfigFile(const std::filesystem::path& config_file);

        /**
         * @brief Removes leading and trailing whitespace from a string
         *
         * @param str The string to trim
         * @return The trimmed string
         */
        std::string trimWhitespace(const std::string& str) const;

        /**
         * @brief Finds a configuration value by section and key
         *
         * Searches for the key in the specified section. If not found and the
         * section is not DEFAULT_SECTION, tries to find the key in DEFAULT_SECTION.
         *
         * @param section The section name
         * @param key The configuration key
         * @return Optional reference to the configuration value if found, empty optional otherwise
         */
        std::optional<std::reference_wrapper<const std::string>> findConfigValue(const std::string& section, const std::string& key) const;

        /**
         * @brief Configuration directory path
         */
        std::string _config_dir;

        /**
         * @brief Configuration data structure: section -> key -> value
         */
        std::map<std::string, std::map<std::string, std::string>> _config_data;

        std::string _module_path;
};

/**
 * @brief Global configuration manager instance
 *
 * Provides a single instance of the ConfigManager that can be accessed
 * from anywhere in the application.
 */
extern ConfigManager g_config_manager;