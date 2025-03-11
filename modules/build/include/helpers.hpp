/**
 * @file helpers.hpp
 * @brief Header file for the build module support functions
 *
 * Defines functions and enumerations for the build module which creates
 * DPM packages according to specification.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */
#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <getopt.h>
#include <cstdlib>
#include <wordexp.h>
#include <dpmdk/include/CommonModuleAPI.hpp>

/**
 * @brief Expands environment variables and tildes in a path
 *
 * Uses wordexp to handle shell-like expansions in paths,
 * including environment variables, tildes, and wildcards.
 *
 * @param path The path string to expand
 * @return The expanded path, or the original path if expansion failed
 */
std::string expand_path(const std::string& path);



