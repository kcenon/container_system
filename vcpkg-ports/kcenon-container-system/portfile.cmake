# kcenon-container-system portfile
# Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO kcenon/container_system
    REF "v${VERSION}"
    SHA512 ca2feee08c7cef41d297c49c4a4eb8559dcb4c680d42e70bc011b8928e88b82e16fab3e54f44e15e3c1e1ce0738950de9e8c55fea6d090cd8d3db628a46fb444
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
