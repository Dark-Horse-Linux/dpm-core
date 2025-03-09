# DPM Development Kit (DPMDK)

Core interface definitions for developing Dark Horse Package Manager modules.

## Purpose

DPMDK provides the essential interface that all DPM modules must implement to be compatible with the DPM module loading system.

## Required Module Implementation

Every DPM module must implement these functions:

```cpp
extern "C" const char* dpm_module_get_version(void);
extern "C" const char* dpm_get_description(void);
extern "C" int dpm_module_execute(const char* command, int argc, char** argv);
```

## Core Functions Available to Modules

The DPM core provides these functions for modules to use:

```cpp
extern "C" const char* dpm_get_config(const char* section, const char* key);
extern "C" void dpm_log(int level, const char* message);
```

### Logging Levels

DPMDK defines the following constants for use with the `dpm_log` function:

```cpp
LOG_FATAL = 0  // Critical errors causing termination
LOG_ERROR = 1  // Errors preventing operation completion
LOG_WARN  = 2  // Warning conditions
LOG_INFO  = 3  // Informational messages
LOG_DEBUG = 4  // Detailed debugging information
```

## Example Usage

```cpp
#include <dpmdk/include/CommonModuleAPI.hpp>

#define MODULE_VERSION "0.1.0"

extern "C" const char* dpm_module_get_version(void) {
    return MODULE_VERSION;
}

extern "C" const char* dpm_get_description(void) {
    return "Example DPM module";
}

extern "C" int dpm_module_execute(const char* command, int argc, char** argv) {
    dpm_log(LOG_INFO, "Module execution started");
    
    const char* config_value = dpm_get_config("section_name", "key_name");
    
    // Module implementation...
    
    return 0;  // Success
}
```

## Reference Module

See the `info` module in the DPM-Core repository for a complete implementation example.

## License

GNU Affero General Public License v3.0
