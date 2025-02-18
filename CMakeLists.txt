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
)

target_include_directories(dpm PRIVATE include)
target_link_libraries(dpm dl)

# Add the info module
add_library(info MODULE modules/info.cpp)
set_target_properties(info PROPERTIES
        PREFIX ""  # Remove lib prefix
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
)
