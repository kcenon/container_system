# targets.cmake - container_system library target definitions
#
# Creates the container_system static library, attaches its sources, applies
# warning policy and platform/SIMD compile flags, configures include paths,
# defines feature compile-definitions, and links third-party dependencies.
#
# Pre-requisites (must be included before this module):
#   - cmake/options.cmake          (option values)
#   - cmake/compile_options.cmake  (apply_container_compile_options,
#                                   apply_container_simd_definitions)
#   - cmake/dependencies.cmake     (Threads, configure_container_common_system_dep)
#   - cmake/sources.cmake          (ALL_FILES list)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(container_system STATIC
    ${ALL_FILES}
)

# Apply warning policy and platform compile flags
apply_container_compile_options(container_system)

# Wire up common_system dependency (creates kcenon::common_system imported target)
configure_container_common_system_dep(container_system)

# Create aliases for consistent naming and messaging system compatibility
add_library(container_system::container_system ALIAS container_system)  # Canonical <package>::<package>
add_library(container_system::container ALIAS container_system)  # Backward compatibility
add_library(ContainerSystem::container ALIAS container_system)  # Backward compatibility
add_library(MessagingSystem::container ALIAS container_system)  # For messaging system compatibility

# Set target properties
set_target_properties(container_system PROPERTIES
    EXPORT_NAME container_system
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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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

if(CONTAINER_USE_MEMORY_POOL)
    target_compile_definitions(container_system PUBLIC CONTAINER_USE_MEMORY_POOL=1)
    message(STATUS "Container: Memory pool enabled")
endif()

if(CONTAINER_ENABLE_COROUTINES)
    target_compile_definitions(container_system PUBLIC CONTAINER_HAS_COROUTINES=1)
    # Check for coroutine compiler support
    include(CheckCXXCompilerFlag)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        check_cxx_compiler_flag("-fcoroutines" COMPILER_HAS_FCOROUTINES)
        if(COMPILER_HAS_FCOROUTINES)
            target_compile_options(container_system PUBLIC -fcoroutines)
        endif()
    endif()
endif()

##################################################
# Dependencies
##################################################

# Link libraries
target_link_libraries(container_system PUBLIC
    Threads::Threads
    kcenon::common_system
)

# C++20 std::format with ostringstream fallback for incomplete implementations
message(STATUS "Container: Using std::format (C++20) with ostringstream fallback")

##################################################
# Platform / SIMD Configuration
##################################################

# Detect compiler SIMD support and add HAS_AVX2 / HAS_SSE42 / HAS_ARM_NEON
# private compile definitions. Re-exports COMPILER_SUPPORTS_AVX2 and
# COMPILER_SUPPORTS_SSE42 to the parent scope for the summary block.
apply_container_simd_definitions(container_system)
