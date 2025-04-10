cmake_minimum_required(VERSION 3.22)
project(info_module)

set(CMAKE_CXX_STANDARD 20)

# Set DPM_ROOT_DIR based on whether this is a standalone build or part of the main build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DPM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
    set(DPM_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Create shared library
add_library(info MODULE
        info.cpp
        src/infoFuncs.cpp
)

# Set output properties
set_target_properties(
        info PROPERTIES
        PREFIX ""
        SUFFIX ".so"
)

# Include directories
target_include_directories(info PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Standalone version - used for debugging
add_executable(info_standalone
        info.cpp
        src/infoFuncs.cpp
)

# Define the BUILD_STANDALONE macro for the standalone build
target_compile_definitions(info_standalone PRIVATE BUILD_STANDALONE)

# Include directories for standalone
target_include_directories(info_standalone PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)

# Set the output name for the standalone executable
set_target_properties(
        info_standalone PROPERTIES
        OUTPUT_NAME "info_debug"
)