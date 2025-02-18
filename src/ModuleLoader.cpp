#include "ModuleLoader.hpp"
#include "dpm_interface.hpp"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>

namespace fs = std::filesystem;

ModuleLoader::ModuleLoader(std::string module_path) : module_path_(std::move(module_path))
{
    if (!module_path_.empty() && module_path_.back() != '/') {
        module_path_ += '/';
    }
}

DPMError ModuleLoader::check_module_path() const
{
    if (!fs::exists(module_path_)) {
        return DPMError::PATH_NOT_FOUND;
    }

    if (!fs::is_directory(module_path_)) {
        return DPMError::PATH_NOT_DIRECTORY;
    }

    try {
        fs::directory_iterator(module_path_);
    } catch (const fs::filesystem_error&) {
        return DPMError::PERMISSION_DENIED;
    }

    return DPMError::SUCCESS;
}

std::string ModuleLoader::get_absolute_module_path() const
{
    try {
        return fs::absolute(module_path_).string();
    } catch (const fs::filesystem_error&) {
        return module_path_; // Return relative path if conversion fails
    }
}

std::pair<std::vector<std::string>, DPMError> ModuleLoader::list_available_modules() const
{
    std::vector<std::string> modules;

    try {
        fs::path absolute_path = fs::absolute(module_path_);
        for (const auto& entry : fs::directory_iterator(absolute_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                // Check if it's a .so file
                if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".so") {
                    // Remove the .so extension
                    modules.push_back(filename.substr(0, filename.size() - 3));
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        return {modules, DPMError::PERMISSION_DENIED};
    }

    return {modules, DPMError::SUCCESS};
}

void* ModuleLoader::load_module(const std::string& module_name) const
{
    // Construct path to module shared object
    std::string module_so_path = module_path_ + module_name + ".so";

    // Load the module
    void* module_handle = dlopen(module_so_path.c_str(), RTLD_LAZY);
    if (!module_handle) {
        std::cerr << "Failed to load module: " << dlerror() << std::endl;
        return nullptr;
    }

    // Clear any existing errors
    dlerror();

    return module_handle;
}

int ModuleLoader::execute_module(void* module_handle, const std::string& command) const
{
    if (!module_handle) {
        std::cerr << "Invalid module handle" << std::endl;
        return 1;
    }

    // Find the execution entry point
    using ExecuteFn = int (*)(const char*, int, char**);
    ExecuteFn execute_fn = (ExecuteFn)dlsym(module_handle, "dpm_module_execute");

    const char* error = dlerror();
    if (error != nullptr) {
        std::cerr << "Failed to find module entry point: " << error << std::endl;
        return 1;
    }

    // Execute the module with just the provided command string
    int result = execute_fn(command.c_str(), 0, nullptr);

    return result;
}