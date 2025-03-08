cmake_minimum_required(VERSION 3.22)
project(dpm)

set(CMAKE_CXX_STANDARD 20)

# Create modules directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)

add_executable(
        dpm
        src/dpm.cpp
        src/ModuleLoader.cpp
        src/dpm_interface.cpp
        src/error.cpp
        src/dpm_interface_helpers.cpp
        src/handlers.cpp
        src/module_interface.cpp
        src/ConfigManager.cpp
        src/Logger.cpp
)

target_include_directories(dpm PRIVATE include)
target_link_libraries(dpm dl)

# Export symbols for dynamic loading
target_link_options(dpm PRIVATE -rdynamic)

# Add the info module with specific source files
add_library(info MODULE
        modules/info/info.cpp
        modules/info/src/infoFuncs.cpp
        # DO NOT include dpmdk sources which have different include requirements
)

set_target_properties(
        info PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
)

# Add include directories for the info module
target_include_directories(info PRIVATE
        include
        modules/info
        modules/info/include  # Add this to find infoFuncs.hpp
)

# Installation rules
install(TARGETS dpm DESTINATION bin)
install(DIRECTORY DESTINATION /etc/dpm/conf.d)
install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/data/"
        DESTINATION /etc/dpm/conf.d
        FILES_MATCHING
        PATTERN "*.conf"
)

# Install all .so files from build/modules to the module path
install(
        DIRECTORY ${CMAKE_BINARY_DIR}/modules/
        DESTINATION /usr/lib/dpm/modules
        FILES_MATCHING PATTERN "*.so"
)