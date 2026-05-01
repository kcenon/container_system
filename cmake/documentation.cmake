# documentation.cmake - Doxygen target wiring
#
# Looks for Doxygen and registers a `container_system_docs` target plus a
# convenience `docs` alias. Invocation is gated on Doxygen being installed
# locally; absence is non-fatal.
#
# NOTE: The full Doxyfile/install layout review for documentation belongs to
# issue #536. This module only relocates the existing logic from the
# top-level CMakeLists.txt — it does not change any Doxygen settings.

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
    # Note: legacy core/, internal/, integration/ paths were removed in the source
    # migration (issue #533). Public headers now live under include/kcenon/container/
    # and sources under src/. Issue #536 will revisit the full Doxyfile layout.
    set(DOXYGEN_INPUT
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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
