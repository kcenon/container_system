# options.cmake - container_system feature toggles and option declarations
#
# This module declares all `option()` and CACHE variables consumed by other
# cmake/ modules. It is the first module included by the top-level
# CMakeLists.txt so every later module can rely on these values being set.

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
option(BUILD_FUZZING "Build libFuzzer fuzz targets" OFF)
option(CONTAINER_USE_MEMORY_POOL "Enable memory pool for small allocations" ON)
option(CONTAINER_ENABLE_COROUTINES "Enable C++20 coroutine-based async API" ON)
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
option(ENABLE_MESSAGING_FEATURES "Enable messaging-specific optimizations" ON)
option(ENABLE_EXTERNAL_INTEGRATION "Enable integration with external systems" ON)
option(ENABLE_PERFORMANCE_METRICS "Enable performance metrics collection" ON)

# C++20 module build option (used by cmake/modules.cmake)
option(CONTAINER_BUILD_MODULES "Build C++20 module version of container_system" OFF)

# Examples build option (declared late in the legacy file; kept here so all
# options live in one place)
option(BUILD_CONTAINER_EXAMPLES "Build container system examples" ON)
