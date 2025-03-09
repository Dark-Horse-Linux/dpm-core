cmake_minimum_required(VERSION 3.22)
project(info_module)

set(CMAKE_CXX_STANDARD 20)

# Set output directory for standalone builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(MODULE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
    set(DPM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
    set(MODULE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
    set(DPM_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Create output directory
file(MAKE_DIRECTORY ${MODULE_OUTPUT_DIR})

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
        LIBRARY_OUTPUT_DIRECTORY "${MODULE_OUTPUT_DIR}"
)

# Include directories
target_include_directories(info PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DPM_ROOT_DIR}
)