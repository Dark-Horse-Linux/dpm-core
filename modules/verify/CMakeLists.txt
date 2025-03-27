cmake_minimum_required(VERSION 3.22)
project(verify_module)

set(CMAKE_CXX_STANDARD 20)

# Set DPM_ROOT_DIR based on whether this is a standalone build or part of the main build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DPM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
    set(DPM_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Create shared library
add_library(verify MODULE
        verify.cpp
        src/verify_commands.cpp
)

# Set output properties
set_target_properties(
        verify PROPERTIES
        PREFIX ""
        SUFFIX ".so"
)

# Include directories
target_include_directories(verify PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Standalone version - used for debugging
add_executable(verify_standalone
        verify.cpp
        src/verify_commands.cpp
)

# Define the BUILD_STANDALONE macro for the standalone build
target_compile_definitions(verify_standalone PRIVATE BUILD_STANDALONE)

# Include directories for standalone
target_include_directories(verify_standalone PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Set the output name for the standalone executable
set_target_properties(
        verify_standalone PROPERTIES
        OUTPUT_NAME "verify_debug"
)