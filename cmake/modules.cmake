# modules.cmake - C++20 module support (Phase 2)
#
# Optional C++20 module library target, gated on CONTAINER_BUILD_MODULES.
# Requires CMake 3.28+ and a module-capable compiler (Clang 16+, GCC 14+,
# MSVC 2022 17.4+). When unavailable the build silently falls back to the
# header-based interface — no fatal error.

if(NOT CONTAINER_BUILD_MODULES)
    return()
endif()

# Check CMake version
if(CMAKE_VERSION VERSION_LESS "3.28")
    message(WARNING "C++20 modules require CMake 3.28+. Disabling module build.")
    # include()d files share the caller's variable scope, so a plain set()
    # here is sufficient to update the value seen by cmake/summary.cmake.
    set(CONTAINER_BUILD_MODULES OFF)
    return()
endif()

message(STATUS "C++20 module build enabled for container_system")

# Create module library target
add_library(container_system_modules)
add_library(kcenon::container_modules ALIAS container_system_modules)

# Set C++20 standard for module target
target_compile_features(container_system_modules PUBLIC cxx_std_20)

# Enable module scanning
set_target_properties(container_system_modules PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

# Add module source files
target_sources(container_system_modules
    PUBLIC FILE_SET CXX_MODULES
    FILES
        # Single module (container_system is small enough for single module)
        src/modules/container.cppm
)

# Include directories for module target
target_include_directories(container_system_modules PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Link common_system modules if available
if(TARGET kcenon::common_modules)
    target_link_libraries(container_system_modules PUBLIC kcenon::common_modules)
    message(STATUS "Container module: Linked with kcenon::common_modules")
elseif(TARGET common_system_modules)
    target_link_libraries(container_system_modules PUBLIC common_system_modules)
    message(STATUS "Container module: Linked with common_system_modules")
else()
    message(WARNING "kcenon.common module not found. Container module may not build correctly.")
endif()

# Compile definitions
target_compile_definitions(container_system_modules PUBLIC
    KCENON_HAS_COMMON_SYSTEM=1
    KCENON_USE_MODULES=1
)

# Link with threading library
target_link_libraries(container_system_modules PUBLIC Threads::Threads)

message(STATUS "C++20 module target: container_system_modules")
