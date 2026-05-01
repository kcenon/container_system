# test.cmake - test, integration test, and fuzzer registration
#
# Wires up unit tests, integration tests, and fuzz targets. enable_testing()
# is called within the conditional blocks (matching the legacy behaviour:
# testing infrastructure only activates when at least one test category is
# requested AND the project is not being built as a submodule).

# Enable testing features if building tests
if(BUILD_TESTS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Container: Unit tests enabled")
endif()

# Enable integration tests if requested
if(CONTAINER_BUILD_INTEGRATION_TESTS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    enable_testing()
    add_subdirectory(integration_tests)
    message(STATUS "Container: Integration tests enabled")
endif()

if(BUILD_FUZZ_TESTS)
    message(STATUS "Container: Fuzz tests enabled")

    # Check if we are compiling with a fuzzer-capable compiler
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        message(WARNING "Fuzzing typically requires Clang. Proceeding, but link might fail if -fsanitize=fuzzer is not supported.")
    endif()

    add_executable(container_fuzzer tests/fuzz/container_fuzzer.cpp)

    # Link against the container library
    target_link_libraries(container_fuzzer PRIVATE container_system)

    # Add fuzzer flags
    # Note: We use -g -O1 for better stack traces and moderate performance during fuzzing
    target_compile_options(container_fuzzer PRIVATE -g -O1 -fsanitize=fuzzer,address,undefined)
    target_link_options(container_fuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

# libFuzzer-based fuzz targets (WORKFLOW-003)
if(BUILD_FUZZING AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    add_subdirectory(fuzz)
    message(STATUS "Container: libFuzzer targets enabled")
endif()
