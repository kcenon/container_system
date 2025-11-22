cmake_minimum_required(VERSION 3.16)

##################################################
# Container System CMakeLists.txt
# 
# Independent container system with advanced C++20 features
# Provides type-safe, serializable data containers.
##################################################

# Project definition
project(container_system
    DESCRIPTION "Advanced C++17 Container System with Thread-Safe Operations and Messaging Integration"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to control whether ContainerSystem is built as a standalone project or as a submodule
# - OFF (default): Builds as standalone project with samples and tests
# - ON: Builds only the core libraries for integration into other projects
option(BUILD_CONTAINERSYSTEM_AS_SUBMODULE "Build ContainerSystem as submodule" OFF)
option(BUILD_DOCUMENTATION "Build Doxygen documentation" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_CONTAINER_SAMPLES "Build container system samples" ON)
option(USE_THREAD_SAFE_OPERATIONS "Enable thread-safe operations" ON)
option(USE_LOCKFREE_BY_DEFAULT "Use lock-free implementations by default" OFF)
option(BUILD_WITH_COMMON_SYSTEM "Enable common_system integration" ON)

if(NOT BUILD_WITH_COMMON_SYSTEM)
    message(FATAL_ERROR "common_system integration is now mandatory for container_system.")
endif()
option(CONTAINER_BUILD_BENCHMARKS "Build container system benchmarks" OFF)
option(CONTAINER_BUILD_INTEGRATION_TESTS "Build container system integration tests" ON)
option(BUILD_FUZZ_TESTS "Build fuzz tests" OFF)
set(COMMON_SYSTEM_ROOT "" CACHE PATH "Path to the common_system repository root")

# Respect global BUILD_INTEGRATION_TESTS flag if set
if(DEFINED BUILD_INTEGRATION_TESTS)
    if(BUILD_INTEGRATION_TESTS)
        set(_CONTAINER_BUILD_IT_VALUE ON)
    else()
        set(_CONTAINER_BUILD_IT_VALUE OFF)
    endif()
    set(CONTAINER_BUILD_INTEGRATION_TESTS ${_CONTAINER_BUILD_IT_VALUE} CACHE BOOL "Build container system integration tests" FORCE)
endif()

# When common_system integration is enabled, samples and tests are now supported
if(BUILD_WITH_COMMON_SYSTEM)
    message(STATUS "Container system building with common_system integration")
endif()

# New messaging system integration options
option(ENABLE_MESSAGING_FEATURES "Enable messaging-specific optimizations" OFF)
option(ENABLE_EXTERNAL_INTEGRATION "Enable integration with external systems" OFF)
option(ENABLE_PERFORMANCE_METRICS "Enable performance metrics collection" OFF)

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

# Note: This project uses manual string formatting instead of std::format
# to ensure compatibility across all C++20 compilers (including older Clang versions)
message(STATUS "Using manual string formatting (no std::format dependency)")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_store.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_store.cpp
)

# Legacy value files removed - using variant-based storage only

# Internal implementation files
set(INTERNAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/container/internal/thread_safe_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/container/internal/thread_safe_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container/internal/simd_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/container/internal/simd_processor.cpp
)

# Integration files (conditional)
set(INTEGRATION_FILES)
if(ENABLE_MESSAGING_FEATURES OR ENABLE_EXTERNAL_INTEGRATION OR ENABLE_PERFORMANCE_METRICS)
    list(APPEND INTEGRATION_FILES
#         ${CMAKE_CURRENT_SOURCE_DIR}/integration/messaging_integration.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/integration/messaging_integration.cpp
    )
    message(STATUS "Container: Integration modules included")
endif()

# Messaging files (domain-specific)
set(MESSAGING_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messaging/message_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/messaging/message_container.cpp
)

# Combine all files
set(ALL_FILES
    ${MAIN_HEADER}
    ${CORE_FILES}
    ${VALUE_FILES}
    ${INTERNAL_FILES}
    ${INTEGRATION_FILES}
    ${MESSAGING_FILES}
)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(container_system STATIC
    ${ALL_FILES}
)

# Suppress warnings inherited from parent project
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(container_system PRIVATE
        -Wno-sign-conversion
        -Wno-shorten-64-to-32
        -Wno-float-equal
        -Wno-implicit-float-conversion
        -Wno-unreachable-code-loop-increment
    )
elseif(MSVC)
    target_compile_options(container_system PRIVATE
        /wd4244  # conversion from 'type1' to 'type2', possible loss of data
        /wd4267  # conversion from 'size_t' to 'type', possible loss of data
        /wd4305  # truncation from 'type1' to 'type2'
        /wd4365  # conversion from 'type1' to 'type2', signed/unsigned mismatch
        /wd4668  # 'symbol' is not defined as a preprocessor macro
    )
endif()

if(BUILD_WITH_COMMON_SYSTEM)
    message(STATUS "Container System: Enabling common_system integration")

    set(_common_system_roots)
    if(COMMON_SYSTEM_ROOT)
        list(APPEND _common_system_roots ${COMMON_SYSTEM_ROOT})
    endif()
    if(DEFINED ENV{COMMON_SYSTEM_ROOT})
        list(APPEND _common_system_roots $ENV{COMMON_SYSTEM_ROOT})
    endif()
    # Priority order: relative paths (sibling directory), then current directory
    list(APPEND _common_system_roots
        ${CMAKE_CURRENT_SOURCE_DIR}/../common_system
        ${CMAKE_CURRENT_SOURCE_DIR}/common_system
    )

    set(COMMON_SYSTEM_INCLUDE_DIR "")
    foreach(_root IN LISTS _common_system_roots)
        if(NOT _root)
            continue()
        endif()

        if(EXISTS ${_root}/include/kcenon/common/patterns/result.h)
            set(COMMON_SYSTEM_INCLUDE_DIR ${_root}/include)
            break()
        elseif(EXISTS ${_root}/kcenon/common/patterns/result.h)
            set(COMMON_SYSTEM_INCLUDE_DIR ${_root})
            break()
        endif()
    endforeach()

    if(NOT COMMON_SYSTEM_INCLUDE_DIR)
        message(FATAL_ERROR
            "common_system integration requested but header 'kcenon/common/patterns/result.h' "
            "was not found. Set COMMON_SYSTEM_ROOT or ensure the dependency is present.")
    endif()

    message(STATUS "Container System: common_system include directory = ${COMMON_SYSTEM_INCLUDE_DIR}")
    target_include_directories(container_system PUBLIC
        $<BUILD_INTERFACE:${COMMON_SYSTEM_INCLUDE_DIR}>
    )
    target_compile_definitions(container_system PUBLIC BUILD_WITH_COMMON_SYSTEM CONTAINER_USE_COMMON_SYSTEM)
endif()

# Create aliases for consistent naming and messaging system compatibility
add_library(ContainerSystem::container ALIAS container_system)
add_library(MessagingSystem::container ALIAS container_system)  # For messaging system compatibility

# Set target properties
set_target_properties(container_system PROPERTIES
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
    $<INSTALL_INTERFACE:include>
)

##################################################
# Compile Definitions
##################################################

# Internal features are always enabled but not exposed
target_compile_definitions(container_system PRIVATE
    CONTAINER_INTERNAL_FEATURES
)

# Conditional compile definitions for new features
if(ENABLE_MESSAGING_FEATURES)
    target_compile_definitions(container_system PUBLIC HAS_MESSAGING_FEATURES)
    message(STATUS "Container: Messaging features enabled")
endif()

if(ENABLE_EXTERNAL_INTEGRATION)
    target_compile_definitions(container_system PUBLIC HAS_EXTERNAL_INTEGRATION)
    message(STATUS "Container: External integration enabled")
endif()

if(ENABLE_PERFORMANCE_METRICS)
    target_compile_definitions(container_system PUBLIC HAS_PERFORMANCE_METRICS)
    message(STATUS "Container: Performance metrics enabled")
endif()

##################################################
# Dependencies
##################################################

# Link libraries
target_link_libraries(container_system PUBLIC
    Threads::Threads
)

# Formatting library configuration: prefer std::format, fall back to fmt
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
check_cxx_source_compiles("
    #include <format>
    int main() {
        auto s = std::format(\"{}\", 42);
        return 0;
    }
" HAS_WORKING_STD_FORMAT)

if(HAS_WORKING_STD_FORMAT)
    # Prefer std::format if available (C++20)
    target_compile_definitions(container_system PUBLIC USE_STD_FORMAT)
    message(STATUS "Container: Using std::format (C++20)")
else()
    # Fall back to fmt library
    find_package(fmt QUIET)
    if(fmt_FOUND AND TARGET fmt::fmt)
        target_link_libraries(container_system PUBLIC fmt::fmt)
        message(STATUS "Container: Using fmt library")
    else()
        message(WARNING "Container: No formatting library available")
        message(WARNING "  Install fmt library or use compiler with std::format support")
    endif()
endif()

##################################################
# Platform-Specific Configuration
##################################################

# Check for SIMD support
include(CheckCXXCompilerFlag)

# AVX2 support
if(NOT APPLE)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        # Note: AVX2 functions use __attribute__((target("avx2"))) for per-function enabling
        # This allows SIMD code to work in Debug mode without global flags
        target_compile_definitions(container_system PRIVATE HAS_AVX2)
    endif()
endif()

# SSE support (x86/x64 only)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        # Note: SSE functions use __attribute__((target("sse2/sse3"))) for per-function enabling
        # This allows SIMD code to work in Debug mode without global flags
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
    )
    # Clang-specific warnings
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(container_system PRIVATE
            -Wno-unused-lambda-capture
        )
    endif()
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
if(BUILD_TESTS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Container: Unit tests enabled")
endif()
# 
# Enable integration tests if requested
if(CONTAINER_BUILD_INTEGRATION_TESTS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    enable_testing()
    add_subdirectory(integration_tests)
    message(STATUS "Container: Integration tests enabled")
endif()

if(BUILD_FUZZ_TESTS)
    message(STATUS "Container: Fuzz tests enabled")
    
    # Check if we are compiling with a fuzzer-capable compiler
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        message(WARNING "Fuzzing typically requires Clang. Proceeding, but link might fail if -fsanitize=fuzzer is not supported.")
    endif()

    add_executable(container_fuzzer tests/fuzz/container_fuzzer.cpp)
    
    # Link against the container library
    target_link_libraries(container_fuzzer PRIVATE container_system)
    
    # Add fuzzer flags
    # Note: We use -g -O1 for better stack traces and moderate performance during fuzzing
    target_compile_options(container_fuzzer PRIVATE -g -O1 -fsanitize=fuzzer,address,undefined)
    target_link_options(container_fuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

##################################################
# Documentation
##################################################

# Find Doxygen for documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Configure Doxygen settings
    set(DOXYGEN_PROJECT_NAME "Container System")
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documents)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    set(DOXYGEN_MARKDOWN_SUPPORT YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_CLASS_GRAPH YES)
    set(DOXYGEN_COLLABORATION_GRAPH YES)

    # Input files and directories
    set(DOXYGEN_INPUT
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/values
        ${CMAKE_CURRENT_SOURCE_DIR}/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
        ${CMAKE_CURRENT_SOURCE_DIR}/examples
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox
    )

    # Exclude patterns
    set(DOXYGEN_EXCLUDE
        ${CMAKE_CURRENT_SOURCE_DIR}/build
        ${CMAKE_CURRENT_SOURCE_DIR}/build_test
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed
    )

    # Create documentation target
    doxygen_add_docs(container_system_docs
        ${DOXYGEN_INPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating Container System API documentation"
    )

    # Add convenience target
    add_custom_target(docs DEPENDS container_system_docs)

    message(STATUS "Doxygen documentation generation enabled")
    message(STATUS "  Run 'make container_system_docs' or 'make docs' to generate documentation")
    message(STATUS "  Output directory: ${DOXYGEN_OUTPUT_DIRECTORY}")
else()
    message(STATUS "Doxygen not found - documentation generation disabled")
endif()

##################################################
# Samples and Examples (only when not building as submodule)
##################################################

if(NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    if(BUILD_CONTAINER_SAMPLES)
#         add_subdirectory(samples)
        message(STATUS "Container samples will be built")
    endif()

    # Build examples (now supported with common_system integration)
    option(BUILD_CONTAINER_EXAMPLES "Build container system examples" ON)

    if(BUILD_CONTAINER_EXAMPLES)
#         add_subdirectory(examples)
        message(STATUS "Container examples will be built")
    endif()
else()
    message(STATUS "Container samples and examples disabled (submodule mode)")
endif()

##################################################
# Installation
##################################################

include(GNUInstallDirs)

# Install adapter headers if BUILD_WITH_COMMON_SYSTEM is enabled
if(BUILD_WITH_COMMON_SYSTEM)
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/container/adapters/common_result_adapter.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/container/adapters
        COMPONENT Development
    )
endif()

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/kcenon
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
)

##################################################
# Benchmarks
##################################################

# Benchmarks temporarily disabled - require migration to variant-based API
# if(CONTAINER_BUILD_BENCHMARKS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
#     add_subdirectory(benchmarks)
#     message(STATUS "Container benchmarks will be built")
# endif()

##################################################
# Summary
##################################################

message(STATUS "Container library configured:")
message(STATUS "  Module Directory: ${CMAKE_CURRENT_SOURCE_DIR}/container")
if(HAS_WORKING_STD_FORMAT)
    message(STATUS "  Dependencies: utilities, std::format (C++20)")
elseif(fmt_FOUND)
    message(STATUS "  Dependencies: utilities, fmt")
else()
    message(STATUS "  Dependencies: utilities")
endif()
message(STATUS "  Features:")
message(STATUS "    - Thread-safe operations: ON")
message(STATUS "    - Variant-based types: ON")
message(STATUS "    - Build samples: ${BUILD_CONTAINER_SAMPLES}")
if(COMPILER_SUPPORTS_AVX2)
    message(STATUS "    - AVX2 SIMD: ON")
elseif(COMPILER_SUPPORTS_SSE42)
    message(STATUS "    - SSE4.2 SIMD: ON")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "    - ARM NEON SIMD: ON")
else()
    message(STATUS "    - SIMD: OFF (scalar fallback)")
endif()
