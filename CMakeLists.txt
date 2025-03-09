cmake_minimum_required(VERSION 3.22)
project(dpm)

set(CMAKE_CXX_STANDARD 20)

# Create modules directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)

# Main DPM executable
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

# Include directories for the main executable
target_include_directories(dpm PRIVATE include)
target_link_libraries(dpm dl)

# Export symbols for dynamic loading
target_link_options(dpm PRIVATE -rdynamic)

# Add the info module by including its CMakeLists.txt
add_subdirectory(modules/info)

# Create a custom target for building all modules
add_custom_target(modules DEPENDS info)

# Installation rules
install(TARGETS dpm DESTINATION bin)
install(DIRECTORY DESTINATION /etc/dpm/conf.d)
install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/data/"
        DESTINATION /etc/dpm/conf.d
        FILES_MATCHING
        PATTERN "*.conf"
)

# Install modules
install(
        DIRECTORY ${CMAKE_BINARY_DIR}/modules/
        DESTINATION /usr/lib/dpm/modules
        FILES_MATCHING PATTERN "*.so"
)