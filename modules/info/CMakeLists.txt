cmake_minimum_required(VERSION 3.22)
project(dpm-info-module VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create output directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)

# Create shared library with the correct naming convention (no prefix)
add_library(info MODULE
        info.cpp
        src/infoFuncs.cpp
)

# Set output properties for the module
set_target_properties(
        info PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
)

# Include directories based on the actual project structure
target_include_directories(info PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/../../dpmdk/include  # Path to DPMDK headers outside the module directory
)

# Installation rules
install(TARGETS info
        LIBRARY DESTINATION /usr/lib/dpm/modules
)