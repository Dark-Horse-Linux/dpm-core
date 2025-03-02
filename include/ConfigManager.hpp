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

class ConfigManager {
    public:
        // Constructor
        ConfigManager( const std::string& config_dir = DPMDefaultPaths::CONFIG_DIR );

        // Load all configuration files from the config directory
        bool loadConfigurations();

        // Get configuration values with type conversion
        const char* getConfigValue(const char* section, const char* key) const;
        std::string getConfigString(const char* section, const char* key, const std::string& defaultValue = "") const;
        int getConfigInt(const char* section, const char* key, int defaultValue = 0) const;
        double getConfigDouble(const char* section, const char* key, double defaultValue = 0.0) const;
        bool getConfigBool(const char* section, const char* key, bool defaultValue = false) const;

        // Check if configuration directory exists
        bool configDirExists() const;

        // Check if a configuration key exists
        bool hasConfigKey(const char* section, const char* key) const;

    private:
        // Default section name to use when none is specified
        static constexpr const char* DEFAULT_SECTION = "MAIN";

        // Parse a single configuration file
        bool parseConfigFile(const std::filesystem::path& config_file);

        // Trim whitespace from a string
        std::string trimWhitespace(const std::string& str) const;

        // Find a key in the given section or in the default section
        std::optional<std::reference_wrapper<const std::string>> findConfigValue(const std::string& section, const std::string& key) const;

        // Configuration directory path
        std::string _config_dir;

        // Configuration data structure: section -> key -> value
        std::map<std::string, std::map<std::string, std::string>> _config_data;
};

// Global configuration manager instance
extern ConfigManager g_config_manager;
