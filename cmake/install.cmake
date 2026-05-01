# install.cmake - install rules, export set, package config generation
#
# Installs the container_system library, public headers, forwarding headers,
# and CMake package configuration files. The behaviour mirrors the legacy
# top-level CMakeLists.txt exactly — no destinations, components, or pattern
# filters were changed by this refactor (issue #535). Issue #536 will revisit
# the install layout.

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CONTAINER_SYSTEM_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/container_system")

# =============================================================================
# Canonical Include Path Configuration
# =============================================================================
# The canonical include path for container_system is:
#     #include <kcenon/container/container.h>   (preferred)
#     #include <container.h>                    (legacy, via forwarding headers)
#
# The following directories are installed:
#   - include/kcenon/container/  (canonical public headers)
#   - include/container/         (forwarding headers for #include <container/...> paths)
#   - core/      (forwarding headers for backward compatibility)
#   - internal/  (internal implementation details)
#   - integration/ (integration adapters)
#   - messaging/ (domain-specific messaging container)
# =============================================================================
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/container.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
)

install(TARGETS container_system
    EXPORT container_system-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Canonical public headers (kcenon ecosystem convention)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# Forwarding headers under include/container/ (for #include <container/...> paths)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/container
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# Backward-compatible forwarding headers were previously installed from root-level
# core/, internal/, integration/, messaging/ directories. Those directories were
# removed in the source migration (issue #533); the corresponding install rule was
# dropped because the directories no longer exist. Issue #536 will revisit the full
# install layout.

install(EXPORT container_system-targets
    FILE container_system-targets.cmake
    NAMESPACE container_system::
    DESTINATION ${CONTAINER_SYSTEM_CMAKE_INSTALL_DIR}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/container_system-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/container_system-config.cmake"
    INSTALL_DESTINATION ${CONTAINER_SYSTEM_CMAKE_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/container_system-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/container_system-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/container_system-config-version.cmake"
    DESTINATION ${CONTAINER_SYSTEM_CMAKE_INSTALL_DIR}
)

export(EXPORT container_system-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/container_system-targets.cmake"
    NAMESPACE container_system::
)
