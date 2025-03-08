/**
 * @file DPMDefaults.hpp
 * @brief Default configuration values for the DPM utility
 *
 * Defines the DPMDefaults structure which provides default configuration values
 * for paths, logging settings, and other system-wide defaults used by the DPM
 * utility when explicit configuration is not provided.
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
#include "LoggingLevels.hpp"

/**
 * @struct DPMDefaults
 * @brief Default configuration values for DPM
 *
 * Provides system-wide default values for paths, logging settings, and
 * other configuration options used when explicit configuration is not provided.
 */
struct DPMDefaults {
    /**
     * @brief Default path to the directory containing DPM modules
     *
     * Directory where DPM looks for module shared objects (.so files)
     * when no explicit module path is provided.
     */
    static const char* const    MODULE_PATH;

    /**
     * @brief Default path to the directory containing configuration files
     *
     * Directory where DPM looks for configuration files (.conf files)
     * when no explicit configuration directory is provided.
     */
    static const char* const    CONFIG_DIR;

    /**
     * @brief Default path to the log file
     *
     * File where DPM writes log messages when logging to file is enabled
     * and no explicit log file path is provided.
     */
    static const char* const    LOG_FILE;

    /**
     * @brief Default setting for whether to write to log file
     *
     * Determines whether DPM writes log messages to a file by default.
     */
    static const bool           write_to_log;

    /**
     * @brief Default log level
     *
     * Determines which log messages are recorded based on their severity.
     * Messages with a level less than or equal to this value are logged.
     */
    static const LoggingLevels  LOG_LEVEL;
};

// Initialize static constants
/**
 * @brief Default module path initialization
 *
 * Sets the default module path to the standard system location.
 */
inline const char * const   DPMDefaults::MODULE_PATH    = "/usr/lib/dpm/modules/";

/**
 * @brief Default configuration directory initialization
 *
 * Sets the default configuration directory to the standard system location.
 */
inline const char * const   DPMDefaults::CONFIG_DIR     = "/etc/dpm/conf.d/";

/**
 * @brief Default log file path initialization
 *
 * Sets the default log file path to the standard system location.
 */
inline const char * const   DPMDefaults::LOG_FILE       = "/var/log/dpm.log";

/**
 * @brief Default write to log setting initialization
 *
 * By default, logging to file is disabled.
 */
inline const bool           DPMDefaults::write_to_log   = false;

/**
 * @brief Default log level initialization
 *
 * By default, only messages with level INFO or lower (FATAL, ERROR, WARN, INFO)
 * are logged, while DEBUG messages are not.
 */
inline const LoggingLevels  DPMDefaults::LOG_LEVEL      = LoggingLevels::INFO;