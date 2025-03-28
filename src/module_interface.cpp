/**
 * @file module_interface.cpp
 * @brief Implementation of the module interface functions
 *
 * Provides the implementation of functions declared in the module interface
 * that are part of the DPM core, such as callback functions available to modules.
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

#include "module_interface.hpp"

extern "C" const char* dpm_get_config(const char* section, const char* key) {
    return g_config_manager.getConfigValue(section, key);
}

extern "C" void dpm_log(int level, const char* message) {
    if (!message) {
        return;
    }

    // Convert integer level to LoggingLevels enum
    LoggingLevels log_level;
    switch (level) {
        case 0:
            log_level = LoggingLevels::FATAL;
            break;
        case 1:
            log_level = LoggingLevels::ERROR;
            break;
        case 2:
            log_level = LoggingLevels::WARN;
            break;
        case 3:
            log_level = LoggingLevels::INFO;
            break;
        case 4:
            log_level = LoggingLevels::DEBUG;
            break;
        default:
            log_level = LoggingLevels::INFO;
            break;
    }

    g_logger.log(log_level, message);
}

extern "C" void dpm_con(int level, const char* message) {
    if (!message) {
        return;
    }

    // Convert integer level to LoggingLevels enum
    LoggingLevels log_level;
    switch (level) {
        case 0:
            log_level = LoggingLevels::FATAL;
        break;
        case 1:
            log_level = LoggingLevels::ERROR;
        break;
        case 2:
            log_level = LoggingLevels::WARN;
        break;
        case 3:
            log_level = LoggingLevels::INFO;
        break;
        case 4:
            log_level = LoggingLevels::DEBUG;
        break;
        default:
            log_level = LoggingLevels::INFO;
        break;
    }

    g_logger.log_console(log_level, message);
}

extern "C" void dpm_set_logging_level(int level) {
    // Convert integer level to LoggingLevels enum
    LoggingLevels log_level;
    switch (level) {
        case 0:
            log_level = LoggingLevels::FATAL;
        break;
        case 1:
            log_level = LoggingLevels::ERROR;
        break;
        case 2:
            log_level = LoggingLevels::WARN;
        break;
        case 3:
            log_level = LoggingLevels::INFO;
        break;
        case 4:
            log_level = LoggingLevels::DEBUG;
        break;
        default:
            log_level = LoggingLevels::INFO;
        break;
    }

    g_logger.setLogLevel(log_level);
}

extern "C" const char* dpm_get_module_path(void) {
    static std::string module_path;
    module_path = g_config_manager.getModulePath();
    return module_path.c_str();
}