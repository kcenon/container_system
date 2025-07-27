cmake_minimum_required(VERSION 3.16)

##################################################
# Container System Project
# Independent, reusable container component
##################################################

project(ContainerSystem 
    VERSION 2.0.0
    DESCRIPTION "Thread-safe serializable container library with advanced features"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_CONTAINER_TESTS "Build container tests" ON)
option(BUILD_CONTAINER_SAMPLES "Build container samples" ON)
option(BUILD_CONTAINER_DOCS "Build container documentation" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

##################################################
# External Dependencies Management
##################################################

# Option to use external thread_system utilities
option(USE_EXTERNAL_THREAD_UTILITIES "Use external thread utilities" ON)

if(USE_EXTERNAL_THREAD_UTILITIES)
    # Try to find external ThreadSystem utilities
    find_path(THREAD_UTILITIES_INCLUDE_DIR
        NAMES utilities/conversion/convert_string.h
        PATHS
            ${CMAKE_PREFIX_PATH}/include/thread_system
            ${CMAKE_CURRENT_SOURCE_DIR}/../thread_system/sources
            ${CMAKE_CURRENT_SOURCE_DIR}/../messaging_system/thread_system/sources
        NO_DEFAULT_PATH
    )
    
    if(THREAD_UTILITIES_INCLUDE_DIR)
        message(STATUS "Found thread utilities at: ${THREAD_UTILITIES_INCLUDE_DIR}")
        set(THREAD_UTILITIES_FOUND TRUE)
    else()
        message(WARNING "Thread utilities not found - some features may be limited")
        set(THREAD_UTILITIES_FOUND FALSE)
    endif()
else()
    set(THREAD_UTILITIES_FOUND FALSE)
endif()

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

# Create library
if(BUILD_SHARED_LIBS)
    add_library(container SHARED ${ALL_FILES})
else()
    add_library(container STATIC ${ALL_FILES})
endif()

# Create namespace alias
add_library(ContainerSystem::container ALIAS container)

# Set target properties
set_target_properties(container PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

##################################################
# Include Directories
##################################################

# Public include directories
target_include_directories(container PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/container_system>
)

# Thread utilities include (if available)
if(THREAD_UTILITIES_FOUND)
    target_include_directories(container PRIVATE
        ${THREAD_UTILITIES_INCLUDE_DIR}
    )
    target_compile_definitions(container PRIVATE
        HAS_THREAD_UTILITIES
    )
endif()

##################################################
# Dependencies
##################################################

# Standard dependencies
target_link_libraries(container PUBLIC 
    fmt::fmt
    Threads::Threads
)

##################################################
# Compile Definitions
##################################################

target_compile_definitions(container PRIVATE
    CONTAINER_INTERNAL_FEATURES
)

# Thread utilities feature flag
if(THREAD_UTILITIES_FOUND)
    target_compile_definitions(container PUBLIC
        CONTAINER_HAS_THREAD_UTILITIES
    )
endif()

##################################################
# Platform-Specific Configuration
##################################################

# Check for SIMD support
include(CheckCXXCompilerFlag)

# AVX2 support (Intel/AMD)
if(NOT APPLE)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(container PRIVATE $<$<CONFIG:Release>:-mavx2>)
        target_compile_definitions(container PRIVATE HAS_AVX2)
    endif()
endif()

# SSE support (x86/x64 only)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        target_compile_options(container PRIVATE $<$<CONFIG:Release>:-msse4.2>)
        target_compile_definitions(container PRIVATE HAS_SSE42)
    endif()
endif()

# ARM NEON support
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    target_compile_definitions(container PRIVATE HAS_ARM_NEON)
endif()

##################################################
# Compiler Options
##################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(container PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-lambda-capture
    )
elseif(MSVC)
    target_compile_options(container PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4996  # deprecated functions
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(container PRIVATE
        _WIN32_WINNT=0x0601
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

##################################################
# Installation Rules
##################################################

include(GNUInstallDirs)

# Install library
install(TARGETS container
    EXPORT ContainerSystemTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(FILES ${MAIN_HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/container_system
)

install(DIRECTORY core values
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/container_system
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.tpp"
)

##################################################
# Package Configuration (for find_package)
##################################################

include(CMakePackageConfigHelpers)

# Create config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ContainerSystemConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ContainerSystemConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ContainerSystem
)

write_basic_package_version_file(
    ContainerSystemConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ContainerSystemConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ContainerSystemConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ContainerSystem
)

install(EXPORT ContainerSystemTargets
    FILE ContainerSystemTargets.cmake
    NAMESPACE ContainerSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ContainerSystem
)

##################################################
# Summary
##################################################

message(STATUS "ContainerSystem configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Thread utilities: ${THREAD_UTILITIES_FOUND}")
message(STATUS "  Tests: ${BUILD_CONTAINER_TESTS}")
message(STATUS "  Samples: ${BUILD_CONTAINER_SAMPLES}")
message(STATUS "  Documentation: ${BUILD_CONTAINER_DOCS}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "  SIMD: ARM NEON")
elseif(COMPILER_SUPPORTS_AVX2)
    message(STATUS "  SIMD: AVX2")
elseif(COMPILER_SUPPORTS_SSE42)
    message(STATUS "  SIMD: SSE4.2")
else()
    message(STATUS "  SIMD: Scalar fallback")
endif()