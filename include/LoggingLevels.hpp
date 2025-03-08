/**
 * @file LoggingLevels.hpp
 * @brief Defines log level enumeration for DPM logging system
 *
 * Provides a standardized set of logging levels used throughout the DPM system
 * to classify messages by severity and control logging verbosity.
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

extern "C" {
    /**
     * @enum LoggingLevels
     * @brief Enumeration of logging severity levels
     *
     * Defines the various severity levels for log messages in the DPM system.
     * Lower values represent higher severity. This allows for filtering log
     * messages based on their importance.
     */
    enum LoggingLevels {
        FATAL = 0,  /**< Critical errors that cause immediate program termination */
        ERROR = 1,  /**< Errors that prevent an operation from completing but allow program to continue */
        WARN  = 2,  /**< Warning conditions that don't prevent operation but indicate potential issues */
        INFO  = 3,  /**< Informational messages about normal program operation */
        DEBUG = 4   /**< Detailed debug information for troubleshooting */
    };
}