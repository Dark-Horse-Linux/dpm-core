/**
 * @file buildFuncs.cpp
 * @brief Implementation of the build module support functions
 *
 * Implements functions for the build module that create DPM packages
 * according to the specification.
 *
 * @copyright Copyright (c) 2025 SILO GROUP LLC
 * @author Chris Punches <chris.punches@silogroup.org>
 *
 * Part of the Dark Horse Linux Package Manager (DPM)
 */

#include "helpers.hpp"

std::string expand_path(const std::string& path) {
    wordexp_t exp_result;
    std::string expanded_path = path;

    // Use wordexp to expand the path
    if (wordexp(path.c_str(), &exp_result, 0) == 0) {
        if (exp_result.we_wordc > 0) {
            expanded_path = exp_result.we_wordv[0];
        }
        wordfree(&exp_result);
    }

    return expanded_path;
}
