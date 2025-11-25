# Version.cmake - Extract version from Git tags
#
# This module extracts version information from Git tags and sets
# the following variables:
#   - VERSION_MAJOR
#   - VERSION_MINOR
#   - VERSION_PATCH
#   - PROJECT_VERSION
#   - GIT_COMMIT_HASH
#
# Usage:
#   include(cmake/Version.cmake)
#   message(STATUS "Version: ${PROJECT_VERSION}")

find_package(Git QUIET)

if(GIT_FOUND)
    # Get version from git describe
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_RESULT
    )

    # Get commit hash
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Parse version from tag (e.g., v2.1.0)
    if(GIT_VERSION MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        set(VERSION_MAJOR ${CMAKE_MATCH_1})
        set(VERSION_MINOR ${CMAKE_MATCH_2})
        set(VERSION_PATCH ${CMAKE_MATCH_3})
        message(STATUS "Version from Git tag: v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    elseif(GIT_VERSION MATCHES "^v([0-9]+)\\.([0-9]+)")
        set(VERSION_MAJOR ${CMAKE_MATCH_1})
        set(VERSION_MINOR ${CMAKE_MATCH_2})
        set(VERSION_PATCH 0)
        message(STATUS "Version from Git tag: v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    else()
        message(STATUS "No version tag found, using default version")
    endif()
endif()

# Default version if not found from Git
if(NOT DEFINED VERSION_MAJOR)
    set(VERSION_MAJOR 2)
    set(VERSION_MINOR 0)
    set(VERSION_PATCH 0)
    set(GIT_COMMIT_HASH "unknown")
    message(STATUS "Using default version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

# Set project version string
set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(PROJECT_VERSION_MAJOR ${VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${VERSION_PATCH})

# Create version header if needed
if(DEFINED CONTAINER_GENERATE_VERSION_HEADER)
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/version.h.in"
        "${CMAKE_BINARY_DIR}/generated/version.h"
        @ONLY
    )
endif()
