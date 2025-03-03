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

// default system configuration
struct DPMDefaults {
    static const char* const    MODULE_PATH;
    static const char* const    CONFIG_DIR;
    static const char* const    LOG_FILE;
    static const bool           write_to_log;
    static const LoggingLevels  LOG_LEVEL;
};

// Initialize static constants
inline const char * const   DPMDefaults::MODULE_PATH    = "/usr/lib/dpm/modules/";

inline const char * const   DPMDefaults::CONFIG_DIR     = "/etc/dpm/conf.d/";

inline const char * const   DPMDefaults::LOG_FILE       = "/var/log/dpm.log";
inline const bool           DPMDefaults::write_to_log   = false;
inline const LoggingLevels  DPMDefaults::LOG_LEVEL      = LoggingLevels::INFO;