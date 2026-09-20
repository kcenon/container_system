# kcenon-container-system portfile
# Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO kcenon/container_system
    REF "v${VERSION}"
    SHA512 680ec3dc12797b26bef55ed8e918abb0a1ff0e91a3c5c9efc7864cccec93a569b580265d57166e4732833a3c6c50183ed77a305f44c1ac77e942b0d1a993204c
    HEAD_REF main
)

# Map vcpkg.json features to CMake options.
# A feature listed multiple times sets several CMake variables at once.
# vcpkg_check_features emits -DXXX=ON when the feature is selected and
# -DXXX=OFF otherwise, so all five flags below default to OFF for a
# minimal install (typical vcpkg consumer use case).
vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        testing  BUILD_TESTS
        testing  CONTAINER_BUILD_INTEGRATION_TESTS
        testing  CONTAINER_BUILD_BENCHMARKS
        samples  BUILD_CONTAINER_SAMPLES
        docs     BUILD_DOCUMENTATION
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DBUILD_WITH_COMMON_SYSTEM=ON
        -DKCENON_WITH_COMMON_SYSTEM=ON
        -DCOMMON_SYSTEM_ROOT=${CURRENT_INSTALLED_DIR}
        -DFETCHCONTENT_FULLY_DISCONNECTED=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME container_system
    CONFIG_PATH lib/cmake/container_system
)

# Remove example/sample executables and empty bin directories
file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/bin/examples"
    "${CURRENT_PACKAGES_DIR}/bin/samples"
    "${CURRENT_PACKAGES_DIR}/debug/bin/examples"
    "${CURRENT_PACKAGES_DIR}/debug/bin/samples"
)
# Clean up empty bin/debug/bin dirs left after removal (no DLLs on non-Windows)
foreach(_bindir IN ITEMS "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
    if(IS_DIRECTORY "${_bindir}")
        file(GLOB _remaining "${_bindir}/*")
        if(NOT _remaining)
            file(REMOVE_RECURSE "${_bindir}")
        endif()
    endif()
endforeach()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
