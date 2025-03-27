cmake_minimum_required(VERSION 3.22)
project(dpm)

set(CMAKE_CXX_STANDARD 20)

# Set binary output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/modules)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
add_subdirectory(modules/info ${CMAKE_BINARY_DIR}/build-modules/info)

# add the build module by including that
add_subdirectory(modules/build ${CMAKE_BINARY_DIR}/build-modules/build)

# add the verify module
add_subdirectory(modules/verify ${CMAKE_BINARY_DIR}/build-modules/verify)

# Create a custom target for building all modules
add_custom_target(modules DEPENDS info build verify)

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
        DIRECTORY ${CMAKE_BINARY_DIR}/bin/modules/
        DESTINATION /usr/lib/dpm/modules
        FILES_MATCHING PATTERN "*.so"
)