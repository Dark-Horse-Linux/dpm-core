#include "ModuleLoader.hpp"

namespace fs = std::filesystem;

ModuleLoader::ModuleLoader(std::string module_path)
{
    try {
        _module_path = fs::absolute(module_path).string();
        if (!_module_path.empty() && _module_path.back() != '/') {
            _module_path += '/';
        }
    } catch (const fs::filesystem_error&) {
        _module_path = module_path;
        if (!_module_path.empty() && _module_path.back() != '/') {
            _module_path += '/';
        }
    }
}

DPMError ModuleLoader::get_module_path(std::string& path) const
{
    path = _module_path;
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::list_available_modules(std::vector<std::string>& modules) const
{
    modules.clear();

    try {
        for (const auto& entry : fs::directory_iterator(_module_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".so") {
                    modules.push_back(filename.substr(0, filename.size() - 3));
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        return DPMError::PERMISSION_DENIED;
    }

    return DPMError::SUCCESS;
}

DPMError ModuleLoader::load_module(const std::string& module_name, void*& module_handle) const
{
    std::string module_so_path = _module_path + module_name + ".so";

    module_handle = dlopen(module_so_path.c_str(), RTLD_LAZY);
    if (!module_handle) {
        return DPMError::MODULE_LOAD_FAILED;
    }

    dlerror();
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::execute_module(const std::string& module_name, const std::string& command) const
{
    void* module_handle;
    DPMError load_error = load_module(module_name, module_handle);

    if (load_error != DPMError::SUCCESS) {
        return load_error;
    }

    std::vector<std::string> missing_symbols;
    DPMError validate_error = validate_module_interface(module_handle, missing_symbols);
    if (validate_error != DPMError::SUCCESS) {
        dlclose(module_handle);
        return DPMError::INVALID_MODULE;
    }

    using ExecuteFn = int (*)(const char*, int, char**);
    ExecuteFn execute_fn = (ExecuteFn)dlsym(module_handle, "dpm_module_execute");

    const char* error = dlerror();
    DPMError result = DPMError::SUCCESS;

    if (error != nullptr) {
        result = DPMError::MODULE_LOAD_FAILED;
    } else {
        execute_fn(command.c_str(), 0, nullptr);
    }

    dlclose(module_handle);
    return result;
}

DPMError ModuleLoader::get_module_version(void* module_handle, std::string& version) const
{
    if (!module_handle) {
        version = "ERROR";
        return DPMError::INVALID_MODULE;
    }

    dlerror();

    using GetVersionFn = const char* (*)();
    GetVersionFn get_version = (GetVersionFn)dlsym(module_handle, "dpm_module_get_version");

    const char* error = dlerror();
    if (error != nullptr) {
        version = "unknown";
        return DPMError::MODULE_LOAD_FAILED;
    }

    const char* ver = get_version();
    version = ver ? ver : "unknown";
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::get_module_description(void* module_handle, std::string& description) const
{
    if (!module_handle) {
        description = "ERROR";
        return DPMError::INVALID_MODULE;
    }

    dlerror();

    using GetDescriptionFn = const char* (*)();
    GetDescriptionFn get_description = (GetDescriptionFn)dlsym(module_handle, "dpm_get_description");

    const char* error = dlerror();
    if (error != nullptr) {
        description = "unknown";
        return DPMError::MODULE_LOAD_FAILED;
    }

    const char* desc = get_description();
    description = desc ? desc : "unknown";
    return DPMError::SUCCESS;
}

DPMError ModuleLoader::validate_module_interface(void* module_handle, std::vector<std::string>& missing_symbols) const
{
    if (!module_handle) {
        return DPMError::INVALID_MODULE;
    }

    missing_symbols.clear();

    size_t num_symbols = module_interface::required_symbols.size();
    for (size_t i = 0; i < num_symbols; i++) {
        dlerror();
        void* sym = dlsym(module_handle, module_interface::required_symbols[i].c_str());
        const char* error = dlerror();

        if (error != nullptr) {
            missing_symbols.push_back(module_interface::required_symbols[i]);
        }
    }

    if (missing_symbols.empty()) {
        return DPMError::SUCCESS;
    }

    return DPMError::INVALID_MODULE;
}