#pragma once
#include <iostream>
#include <getopt.h>
#include <vector>
#include "error.hpp"
#include <dlfcn.h>
#include <iomanip>
#include <filesystem>
#include "ModuleLoader.hpp"

/*
 *
 *  DPM Interface methods.  These are wrappers of DPM functionality that are meant to handle user view, turning
 *  error codes into human-presentable information, etc.  Features are defined internally, these will only ever be
 *  wrappers of existing features to provide the human/cli interface.
 *
 */

// check if the module path exists
int main_check_module_path(const ModuleLoader& loader);

// list the modules
int main_list_modules(const ModuleLoader& loader);

// data structure for supplied arguments
struct CommandArgs {
    std::string module_path;
    std::string module_name;
    std::string command;  // All arguments combined into a single command string
};

// parser for populating data structure for supplied arguments
CommandArgs parse_args(int argc, char* argv[]);

// pairs DPMErrors to error messages, prints those error messages, and returns
int print_error(DPMError error, const std::string& module_name, const std::string& module_path);