# documentation.cmake - Doxygen target wiring
#
# Looks for Doxygen and registers a `container_system_docs` target plus a
# convenience `docs` alias. Invocation is gated on Doxygen being installed
# locally; absence is non-fatal.
#
# Input list mirrors the canonical layout used by the standalone Doxyfile:
# public headers live under include/kcenon/container/ and implementations
# under src/. Free-form documentation pages live under docs/*.dox.

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

    # Input files and directories.
    # Restrict to the canonical source roots so generated docs do not pick up
    # build artifacts or unrelated repo metadata. The standalone Doxyfile uses
    # the same set of inputs.
    set(DOXYGEN_INPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/examples
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/mainpage.dox
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/tutorial_containers.dox
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/tutorial_serialization.dox
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/tutorial_integration.dox
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/faq.dox
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/troubleshooting.dox
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
