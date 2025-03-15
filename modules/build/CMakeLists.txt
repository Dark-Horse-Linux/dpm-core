cmake_minimum_required(VERSION 3.22)
project(build_module)

set(CMAKE_CXX_STANDARD 20)

# Set DPM_ROOT_DIR based on whether this is a standalone build or part of the main build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DPM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
    set(DPM_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Module version - used by DPM
add_library(build MODULE
        build.cpp
        src/helpers.cpp
        src/cli_parsers.cpp
        src/commands.cpp
        src/package_staging.cpp
)

# Set output properties
set_target_properties(
        build PROPERTIES
        PREFIX ""
        SUFFIX ".so"
)

# Include directories
target_include_directories(build PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Link with filesystem library
target_link_libraries(build stdc++fs)

# Standalone version - used for debugging
add_executable(build_standalone
        build.cpp
        src/helpers.cpp
        src/cli_parsers.cpp
        src/commands.cpp
        src/package_staging.cpp
)

# Define the BUILD_STANDALONE macro for the standalone build
target_compile_definitions(build_standalone PRIVATE BUILD_STANDALONE)

# Include directories for standalone
target_include_directories(build_standalone PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Link with filesystem library for standalone
target_link_libraries(build_standalone stdc++fs)

# Set the output name for the standalone executable
set_target_properties(
        build_standalone PROPERTIES
        OUTPUT_NAME "build_debug"
)