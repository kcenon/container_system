# install.cmake - install rules, export set, package config generation
#
# Installs the container_system library, public headers, forwarding headers,
# and CMake package configuration files.

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
#   - include/kcenon/container/  (canonical public headers, kcenon ecosystem convention)
#   - include/container/         (deprecated forwarding headers; see DEPRECATION note below)
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

# Canonical public headers (kcenon ecosystem convention).
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# Deprecated forwarding headers under include/container/ (for #include <container/...>
# paths). Carries a #pragma message deprecation notice (issue #534, PR #539). This
# install rule will be dropped together with the directory itself in v1.2.0.
# TODO(v1.2.0): remove the include/container/ install rule and the directory.
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/container
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

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
