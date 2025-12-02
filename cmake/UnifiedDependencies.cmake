#[[
UnifiedDependencies.cmake - Unified dependency resolution for container_system

This module provides a consistent interface for finding dependencies across
different configurations:
  1. Cache variable (e.g., COMMON_SYSTEM_ROOT)
  2. Environment variable (e.g., $ENV{COMMON_SYSTEM_ROOT})
  3. Sibling directory (e.g., ../common_system)
  4. Subdirectory (e.g., ./common_system)
  5. FetchContent fallback (if UNIFIED_ALLOW_FETCHCONTENT is ON)

Usage:
  include(cmake/UnifiedDependencies.cmake)
  unified_find_dependency(common_system REQUIRED)
  target_link_libraries(my_target PUBLIC ${common_system_TARGET})

Options:
  UNIFIED_ALLOW_FETCHCONTENT - Enable FetchContent fallback (default: ON)

Author: kcenon
License: MIT
]]

include(FetchContent)

# Global option for FetchContent fallback
option(UNIFIED_ALLOW_FETCHCONTENT "Allow FetchContent fallback for dependencies" ON)

# GitHub repository URLs for FetchContent
set(UNIFIED_COMMON_SYSTEM_GIT_URL "https://github.com/kcenon/common_system.git" CACHE STRING "Git URL for common_system")
set(UNIFIED_COMMON_SYSTEM_GIT_TAG "main" CACHE STRING "Git tag/branch for common_system")

#[[
unified_find_dependency(<name> [REQUIRED])

Find a dependency using multiple resolution strategies.

Arguments:
  <name>    - The dependency name (e.g., common_system)
  REQUIRED  - If specified, failure to find the dependency is a fatal error

Output variables:
  ${name}_FOUND           - TRUE if dependency was found
  ${name}_INCLUDE_DIR     - Include directory for the dependency
  ${name}_TARGET          - Target name to link against (if applicable)
  ${name}_SOURCE_DIR      - Source directory (for FetchContent)
]]
macro(unified_find_dependency _dep_name)
    # Parse arguments
    set(_ufd_options REQUIRED)
    set(_ufd_one_value_args)
    set(_ufd_multi_value_args)
    cmake_parse_arguments(_ufd "${_ufd_options}" "${_ufd_one_value_args}" "${_ufd_multi_value_args}" ${ARGN})

    # Initialize result variables
    set(${_dep_name}_FOUND FALSE)
    set(${_dep_name}_INCLUDE_DIR "")
    set(${_dep_name}_TARGET "")
    set(${_dep_name}_SOURCE_DIR "")

    # Convert dependency name to uppercase for variables
    string(TOUPPER "${_dep_name}" _dep_name_upper)

    message(STATUS "UnifiedDependencies: Searching for ${_dep_name}...")

    # Build list of search paths (in priority order)
    set(_search_roots)

    # 1. Cache variable (highest priority)
    if(${_dep_name_upper}_ROOT)
        list(APPEND _search_roots "${${_dep_name_upper}_ROOT}")
        message(STATUS "  - Cache variable ${_dep_name_upper}_ROOT: ${${_dep_name_upper}_ROOT}")
    endif()

    # 2. Environment variable
    if(DEFINED ENV{${_dep_name_upper}_ROOT})
        list(APPEND _search_roots "$ENV{${_dep_name_upper}_ROOT}")
        message(STATUS "  - Environment variable ${_dep_name_upper}_ROOT: $ENV{${_dep_name_upper}_ROOT}")
    endif()

    # 3. Sibling directory
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${_dep_name}")
        list(APPEND _search_roots "${CMAKE_CURRENT_SOURCE_DIR}/../${_dep_name}")
        message(STATUS "  - Sibling directory: ${CMAKE_CURRENT_SOURCE_DIR}/../${_dep_name}")
    endif()

    # 4. Subdirectory
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dep_name}")
        list(APPEND _search_roots "${CMAKE_CURRENT_SOURCE_DIR}/${_dep_name}")
        message(STATUS "  - Subdirectory: ${CMAKE_CURRENT_SOURCE_DIR}/${_dep_name}")
    endif()

    # Define header files to search for based on dependency
    if("${_dep_name}" STREQUAL "common_system")
        set(_header_patterns
            "include/kcenon/common/patterns/result.h"
            "kcenon/common/patterns/result.h"
        )
    else()
        # Generic header pattern for other dependencies
        set(_header_patterns
            "include/${_dep_name}.h"
            "${_dep_name}.h"
        )
    endif()

    # Search for the dependency
    foreach(_root IN LISTS _search_roots)
        if(NOT _root OR ${_dep_name}_FOUND)
            continue()
        endif()

        foreach(_pattern IN LISTS _header_patterns)
            if(EXISTS "${_root}/${_pattern}")
                # Determine include directory based on pattern
                if(_pattern MATCHES "^include/")
                    set(${_dep_name}_INCLUDE_DIR "${_root}/include")
                else()
                    set(${_dep_name}_INCLUDE_DIR "${_root}")
                endif()
                set(${_dep_name}_FOUND TRUE)
                set(${_dep_name}_SOURCE_DIR "${_root}")
                message(STATUS "  Found ${_dep_name} at: ${_root}")
                message(STATUS "  Include directory: ${${_dep_name}_INCLUDE_DIR}")
                break()
            endif()
        endforeach()

        if(${_dep_name}_FOUND)
            break()
        endif()
    endforeach()

    # FetchContent fallback
    if(NOT ${_dep_name}_FOUND AND UNIFIED_ALLOW_FETCHCONTENT)
        message(STATUS "  ${_dep_name} not found locally, attempting FetchContent...")

        # Get FetchContent URL and tag
        # Note: In CMake macros, variable comparisons must use quoted form "${var}"
        if("${_dep_name}" STREQUAL "common_system")
            set(_git_url "${UNIFIED_COMMON_SYSTEM_GIT_URL}")
            set(_git_tag "${UNIFIED_COMMON_SYSTEM_GIT_TAG}")
        else()
            message(STATUS "  No FetchContent configuration for ${_dep_name}")
            set(_git_url "")
        endif()

        if(_git_url)
            FetchContent_Declare(
                ${_dep_name}
                GIT_REPOSITORY ${_git_url}
                GIT_TAG ${_git_tag}
                GIT_SHALLOW TRUE
            )

            FetchContent_GetProperties(${_dep_name})
            if(NOT ${_dep_name}_POPULATED)
                message(STATUS "  Fetching ${_dep_name} from ${_git_url}...")
                FetchContent_MakeAvailable(${_dep_name})
            endif()

            # Set variables after fetch
            set(${_dep_name}_FOUND TRUE)
            set(${_dep_name}_SOURCE_DIR "${${_dep_name}_SOURCE_DIR}")

            # Determine include directory for fetched content
            if(EXISTS "${${_dep_name}_SOURCE_DIR}/include")
                set(${_dep_name}_INCLUDE_DIR "${${_dep_name}_SOURCE_DIR}/include")
            else()
                set(${_dep_name}_INCLUDE_DIR "${${_dep_name}_SOURCE_DIR}")
            endif()

            message(STATUS "  Successfully fetched ${_dep_name}")
            message(STATUS "  Source directory: ${${_dep_name}_SOURCE_DIR}")
            message(STATUS "  Include directory: ${${_dep_name}_INCLUDE_DIR}")
        endif()
    endif()

    # Handle required dependencies
    if(NOT ${_dep_name}_FOUND)
        set(_error_msg
            "${_dep_name} not found. Searched in:\n"
        )
        foreach(_root IN LISTS _search_roots)
            string(APPEND _error_msg "  - ${_root}\n")
        endforeach()
        string(APPEND _error_msg
            "\nTo resolve, you can:\n"
            "  1. Set ${_dep_name_upper}_ROOT cache variable\n"
            "  2. Set ${_dep_name_upper}_ROOT environment variable\n"
            "  3. Place ${_dep_name} in sibling or subdirectory\n"
        )
        if(UNIFIED_ALLOW_FETCHCONTENT)
            string(APPEND _error_msg
                "  4. FetchContent fallback failed (check network/git configuration)\n"
            )
        else()
            string(APPEND _error_msg
                "  4. Enable UNIFIED_ALLOW_FETCHCONTENT for automatic download\n"
            )
        endif()

        if(_ufd_REQUIRED)
            message(FATAL_ERROR "${_error_msg}")
        else()
            message(WARNING "${_error_msg}")
        endif()
    endif()

    # Set target variable for convenience
    if(${_dep_name}_FOUND)
        # For header-only libraries, we don't have a target
        # The consumer should use target_include_directories
        set(${_dep_name}_TARGET "")
    endif()

    # Clean up local variables
    unset(_search_roots)
    unset(_header_patterns)
    unset(_pattern)
    unset(_root)
    unset(_git_url)
    unset(_git_tag)
    unset(_error_msg)
endmacro()
