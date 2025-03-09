# DPM Module Development

Modules for the Dark Horse Package Manager (DPM) can be developed independently of the core system. The `info` module in this repository provides a reference implementation.

## Required Dependencies

Modules must include the DPMDK headers:

```cpp
#include <dpmdk/include/CommonModuleAPI.hpp>
```

## Required Functions

Every module must implement these three functions:

```cpp
extern "C" const char* dpm_module_get_version(void);
extern "C" const char* dpm_get_description(void);
extern "C" int dpm_module_execute(const char* command, int argc, char** argv);
```

See `modules/info/info.cpp` for a working implementation.

## Building

See the standalone CMakeLists.txt in the `modules/info` directory as a reference for building independent modules.

## Configuration

Standard configuration location: `/etc/dpm/conf.d/`