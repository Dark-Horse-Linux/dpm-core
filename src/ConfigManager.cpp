/**
 * @file ConfigManager.cpp
 * @brief Implementation of the configuration management system
 *
 * Implements the ConfigManager class methods for loading and parsing configuration
 * files from the /etc/dpm/conf.d/ directory, and providing access to configuration
 * values through a consistent interface. Handles INI-style files with sections
 * and key-value pairs.
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

#include "ConfigManager.hpp"

// Global configuration manager instance
ConfigManager g_config_manager;

ConfigManager::ConfigManager()
    : _config_dir(DPMDefaults::CONFIG_DIR)
{
    // Ensure the config directory ends with a slash
    if (!_config_dir.empty() && _config_dir.back() != '/') {
        _config_dir += '/';
    }
}

void ConfigManager::setConfigDir(const std::string& config_dir)
{
    _config_dir = config_dir;

    // Ensure the config directory ends with a slash
    if (!_config_dir.empty() && _config_dir.back() != '/') {
        _config_dir += '/';
    }
}

std::string ConfigManager::getConfigDir() const
{
    return _config_dir;
}

std::string ConfigManager::trimWhitespace(const std::string& str) const
{
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);

    // Return empty string if there are only whitespace characters
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool ConfigManager::configDirExists() const
{
    DIR* dir = opendir(_config_dir.c_str());
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

bool ConfigManager::loadConfigurations()
{
    // Clear existing configuration data
    _config_data.clear();

    // Ensure DEFAULT_SECTION exists in the configuration data
    _config_data[DEFAULT_SECTION] = std::map<std::string, std::string>();

    // Check if the configuration directory exists
    if (!configDirExists()) {
        std::cerr << "Warning: Configuration directory does not exist: " << _config_dir << std::endl;
        return false;
    }

    // Open the directory
    DIR* dir = opendir(_config_dir.c_str());
    if (!dir) {
        std::cerr << "Error: Failed to open configuration directory: " << _config_dir << std::endl;
        return false;
    }

    bool success = true;
    struct dirent* entry;

    // Iterate through directory entries
    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;

        // Skip . and .. directories
        if (filename == "." || filename == "..") {
            continue;
        }

        // Check if file ends with .conf
        if (filename.length() >= 5 &&
            filename.substr(filename.length() - 5) == ".conf") {

            std::string filepath = _config_dir + filename;
            if (!parseConfigFile(filepath)) {
                std::cerr << "Warning: Failed to parse config file: " << filepath << std::endl;
                success = false;
            }
        }
    }

    closedir(dir);
    return success;
}

bool ConfigManager::parseConfigFile(const std::filesystem::path& config_file)
{
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << config_file << std::endl;
        return false;
    }

    std::string line;
    std::string current_section = DEFAULT_SECTION;

    // Process each line in the file
    while (std::getline(file, line)) {
        // Trim whitespace
        line = trimWhitespace(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Check for section header
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.length() - 2);

            // Trim whitespace from section name
            current_section = trimWhitespace(current_section);

            // Skip empty section names, use default instead
            if (current_section.empty()) {
                current_section = DEFAULT_SECTION;
            }

            // Ensure section exists in the map
            if (_config_data.find(current_section) == _config_data.end()) {
                _config_data[current_section] = std::map<std::string, std::string>();
            }

            continue;
        }

        // Parse key-value pair
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = trimWhitespace(line.substr(0, delimiter_pos));
            std::string value = trimWhitespace(line.substr(delimiter_pos + 1));

            // Skip empty keys
            if (key.empty()) {
                continue;
            }

            // Store in configuration map
            _config_data[current_section][key] = value;
        }
    }

    file.close();
    return true;
}

std::optional<std::reference_wrapper<const std::string>> ConfigManager::findConfigValue(
    const std::string& section, const std::string& key) const
{
    // Check if section exists
    auto section_it = _config_data.find(section);
    if (section_it != _config_data.end()) {
        // Check if key exists in section
        auto key_it = section_it->second.find(key);
        if (key_it != section_it->second.end()) {
            return key_it->second;
        }
    }

    // If section is not DEFAULT_SECTION and key was not found,
    // try looking in the DEFAULT_SECTION
    if (section != DEFAULT_SECTION) {
        auto default_section_it = _config_data.find(DEFAULT_SECTION);
        if (default_section_it != _config_data.end()) {
            auto default_key_it = default_section_it->second.find(key);
            if (default_key_it != default_section_it->second.end()) {
                return default_key_it->second;
            }
        }
    }

    // Key not found in specified section or DEFAULT_SECTION
    return std::nullopt;
}

bool ConfigManager::hasConfigKey(const char* section, const char* key) const
{
    if (!key) {
        return false;
    }

    // Use the default section if none is provided
    std::string section_str = section ? section : DEFAULT_SECTION;
    std::string key_str(key);

    return findConfigValue(section_str, key_str).has_value();
}

const char* ConfigManager::getConfigValue(const char* section, const char* key) const
{
    if (!key) {
        return nullptr;
    }

    // Use the default section if none is provided
    std::string section_str = section ? section : DEFAULT_SECTION;
    std::string key_str(key);

    auto value_opt = findConfigValue(section_str, key_str);
    if (value_opt.has_value()) {
        return value_opt.value().get().c_str();
    }

    return nullptr;
}

std::string ConfigManager::getConfigString(const char* section, const char* key, const std::string& defaultValue) const
{
    const char* value = getConfigValue(section, key);
    return value ? value : defaultValue;
}

int ConfigManager::getConfigInt(const char* section, const char* key, int defaultValue) const
{
    const char* value = getConfigValue(section, key);
    if (!value) {
        return defaultValue;
    }

    char* endptr;
    int result = strtol(value, &endptr, 10);

    // If conversion failed or didn't consume the entire string, return default
    if (*endptr != '\0') {
        return defaultValue;
    }

    return result;
}

double ConfigManager::getConfigDouble(const char* section, const char* key, double defaultValue) const
{
    const char* value = getConfigValue(section, key);
    if (!value) {
        return defaultValue;
    }

    char* endptr;
    double result = strtod(value, &endptr);

    // If conversion failed or didn't consume the entire string, return default
    if (*endptr != '\0') {
        return defaultValue;
    }

    return result;
}

bool ConfigManager::getConfigBool(const char* section, const char* key, bool defaultValue) const
{
    const char* value = getConfigValue(section, key);
    if (!value) {
        return defaultValue;
    }

    std::string value_lower = value;
    std::transform(value_lower.begin(), value_lower.end(), value_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Check for common true values
    if (value_lower == "true" || value_lower == "yes" || value_lower == "1" ||
        value_lower == "on" || value_lower == "enabled") {
        return true;
    }

    // Check for common false values
    if (value_lower == "false" || value_lower == "no" || value_lower == "0" ||
        value_lower == "off" || value_lower == "disabled") {
        return false;
    }

    // If not recognized, return default
    return defaultValue;
}

void ConfigManager::setModulePath(const char * module_path) {
    _module_path = module_path;
}

const char * ConfigManager::getModulePath() const {
    return _module_path.c_str();
}