cmake_minimum_required(VERSION 3.16)

##################################################
# Container System CMakeLists.txt
# 
# Independent container system with advanced C++20 features
# Provides type-safe, serializable data containers.
##################################################

# Project definition
project(container_system 
    VERSION 2.0.0
    DESCRIPTION "Advanced C++20 Container System with Thread-Safe Operations"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(USE_UNIT_TEST "Use unit test" ON)
option(BUILD_CONTAINER_SAMPLES "Build container system samples" ON)
option(USE_THREAD_SAFE_OPERATIONS "Enable thread-safe operations" ON)
option(USE_LOCKFREE_BY_DEFAULT "Use lock-free implementations by default" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
elseif(APPLE)
    add_definitions(-DAPPLE_PLATFORM)
endif()

# Find required packages
find_package(Threads REQUIRED)

# vcpkg integration
set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "VCPKG target triplet")
find_package(fmt CONFIG REQUIRED)

##################################################
# Source Files Configuration
##################################################

# Main include file
set(MAIN_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/container.h
)

# Core files (public API)
set(CORE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.cpp
)

# Value files (public API)
file(GLOB VALUE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.tpp
)

# Internal implementation files
set(INTERNAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.cpp
)

# Combine all files
set(ALL_FILES 
    ${MAIN_HEADER}
    ${CORE_FILES} 
    ${VALUE_FILES} 
    ${INTERNAL_FILES}
)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(container_system STATIC 
    ${ALL_FILES}
)

# Create alias for consistent naming
add_library(ContainerSystem::container ALIAS container_system)

# Set target properties
set_target_properties(container_system PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

##################################################
# Include Directories
##################################################

# Public include directories (for consumers of this library)
target_include_directories(container_system PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:include>
)

##################################################
# Compile Definitions
##################################################

# Internal features are always enabled but not exposed
target_compile_definitions(container_system PRIVATE
    CONTAINER_INTERNAL_FEATURES
)

##################################################
# Dependencies
##################################################

# fmt library for formatting
target_link_libraries(container_system PUBLIC 
    fmt::fmt
    Threads::Threads
)

##################################################
# Platform-Specific Configuration
##################################################

# Check for SIMD support
include(CheckCXXCompilerFlag)

# AVX2 support
if(NOT APPLE)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(container_system PRIVATE $<$<CONFIG:Release>:-mavx2>)
        target_compile_definitions(container_system PRIVATE HAS_AVX2)
    endif()
endif()

# SSE support (x86/x64 only)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        target_compile_options(container_system PRIVATE $<$<CONFIG:Release>:-msse4.2>)
        target_compile_definitions(container_system PRIVATE HAS_SSE42)
    endif()
endif()

# ARM NEON support
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    target_compile_definitions(container_system PRIVATE HAS_ARM_NEON)
endif()

##################################################
# Compiler Options
##################################################

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(container_system PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-lambda-capture
    )
elseif(MSVC)
    target_compile_options(container_system PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4996  # deprecated functions
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(container_system PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7 or later
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

##################################################
# Installation Rules (when not building as submodule)
##################################################

# Installation is handled by parent CMakeLists.txt

# Export configuration is handled by parent CMakeLists.txt

##################################################
# Testing Support
##################################################

# Enable testing features if building tests
if(BUILD_UNIT_TESTS AND NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    enable_testing()
    # Tests are handled by the main unittest directory
endif()

##################################################
# Documentation
##################################################

# Find Doxygen for documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    
    doxygen_add_docs(container_system_docs
        ${CMAKE_CURRENT_SOURCE_DIR}/container
        COMMENT "Generating container documentation"
    )
endif()

##################################################
# Summary
##################################################

message(STATUS "Container library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Module Directory: ${CMAKE_CURRENT_SOURCE_DIR}/container")
message(STATUS "  Dependencies: utilities, fmt")
message(STATUS "  Features:")
message(STATUS "    - Thread-safe operations: ON")
message(STATUS "    - Variant-based types: ON")
if(COMPILER_SUPPORTS_AVX2)
    message(STATUS "    - AVX2 SIMD: ON")
elseif(COMPILER_SUPPORTS_SSE42)
    message(STATUS "    - SSE4.2 SIMD: ON")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "    - ARM NEON SIMD: ON")
else()
    message(STATUS "    - SIMD: OFF (scalar fallback)")
endif()