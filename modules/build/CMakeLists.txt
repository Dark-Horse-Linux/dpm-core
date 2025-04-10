cmake_minimum_required(VERSION 3.22)
project(build_module)

set(CMAKE_CXX_STANDARD 20)

# Set DPM_ROOT_DIR based on whether this is a standalone build or part of the main build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
set(DPM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
set(DPM_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find LibArchive
find_package(LibArchive REQUIRED)

# Find GPGME
find_path(GPGME_INCLUDE_DIR gpgme.h)
find_library(GPGME_LIBRARY NAMES gpgme)

if(NOT GPGME_INCLUDE_DIR OR NOT GPGME_LIBRARY)
message(FATAL_ERROR "GPGME library not found. Please install libgpgme-dev or equivalent package.")
endif()

# Module version - used by DPM
add_library(build MODULE
build.cpp
src/helpers.cpp
src/cli_parsers.cpp
src/commands.cpp
src/staging.cpp
src/signing.cpp
src/checksums.cpp
src/metadata.cpp
src/sealing.cpp
        src/archive_reader.cpp
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
${OPENSSL_INCLUDE_DIR}
${LibArchive_INCLUDE_DIRS}
${GPGME_INCLUDE_DIR}
)

# Link with libraries
target_link_libraries(build stdc++fs ${OPENSSL_LIBRARIES} ${LibArchive_LIBRARIES} ${GPGME_LIBRARY})

# Standalone version - used for debugging
add_executable(build_standalone
build.cpp
src/helpers.cpp
src/cli_parsers.cpp
src/commands.cpp
src/staging.cpp
src/signing.cpp
src/checksums.cpp
src/metadata.cpp
src/sealing.cpp
        src/archive_reader.cpp
)

# Define the BUILD_STANDALONE macro for the standalone build
target_compile_definitions(build_standalone PRIVATE BUILD_STANDALONE)

# Include directories for standalone
target_include_directories(build_standalone PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include
${DPM_ROOT_DIR}
${OPENSSL_INCLUDE_DIR}
${LibArchive_INCLUDE_DIRS}
${GPGME_INCLUDE_DIR}
)

# Link with libraries for standalone
target_link_libraries(build_standalone stdc++fs ${OPENSSL_LIBRARIES} ${LibArchive_LIBRARIES} ${GPGME_LIBRARY})

# Set the output name for the standalone executable
set_target_properties(
build_standalone PROPERTIES
OUTPUT_NAME "build_debug"
)