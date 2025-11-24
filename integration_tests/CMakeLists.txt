# Container System Integration Tests CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Enable testing
enable_testing()

# Find required packages for testing
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # If GTest is not found, try to use FetchContent to download it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_LIBRARIES gtest gtest_main)
    set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include)
else()
    set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
    set(GTEST_INCLUDE_DIRS "")
endif()

# Integration test source files
set(INTEGRATION_TEST_SOURCES
    framework/test_environment_validation.cpp
    # TODO: REACT-001 - requires legacy API migration (optimized_value methods)
    # scenarios/container_lifecycle_test.cpp
    # scenarios/value_operations_test.cpp
    # performance/serialization_performance_test.cpp
    # failures/error_handling_test.cpp
)

# Create test executables
foreach(TEST_SOURCE ${INTEGRATION_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    get_filename_component(TEST_DIR ${TEST_SOURCE} DIRECTORY)

    # Create unique target name with directory prefix
    string(REPLACE "/" "_" TEST_TARGET "${TEST_DIR}_${TEST_NAME}")

    add_executable(${TEST_TARGET} ${TEST_SOURCE})

    # Link with container system library
    target_link_libraries(${TEST_TARGET}
        container_system
        ${GTEST_LIBRARIES}
        Threads::Threads
    )

    # Include directories
    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/framework
        ${GTEST_INCLUDE_DIRS}
    )

    # Set compile definitions based on features
    if(ENABLE_MESSAGING_FEATURES)
        target_compile_definitions(${TEST_TARGET} PRIVATE HAS_MESSAGING_FEATURES)
    endif()

    if(ENABLE_PERFORMANCE_METRICS)
        target_compile_definitions(${TEST_TARGET} PRIVATE HAS_PERFORMANCE_METRICS)
    endif()

    if(ENABLE_EXTERNAL_INTEGRATION)
        target_compile_definitions(${TEST_TARGET} PRIVATE HAS_EXTERNAL_INTEGRATION)
    endif()

    # Add as CTest
    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

    # Set test properties
    set_tests_properties(${TEST_TARGET} PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Label tests by category
    if(TEST_DIR STREQUAL "framework")
        set_tests_properties(${TEST_TARGET} PROPERTIES LABELS "framework;integration;environment")
    elseif(TEST_DIR STREQUAL "scenarios")
        set_tests_properties(${TEST_TARGET} PROPERTIES LABELS "scenarios;integration")
    elseif(TEST_DIR STREQUAL "performance")
        set_tests_properties(${TEST_TARGET} PROPERTIES
            LABELS "performance;integration"
            TIMEOUT 600
        )
    elseif(TEST_DIR STREQUAL "failures")
        set_tests_properties(${TEST_TARGET} PROPERTIES LABELS "failures;integration")
    endif()
endforeach()

# Note: Combined integration test runner removed due to multiple main() definitions
# Each test file has its own main() function and should be run separately
# Use run_all_integration target or ctest to run all tests

# Test coverage configuration (if supported)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(TEST_SOURCE ${INTEGRATION_TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        get_filename_component(TEST_DIR ${TEST_SOURCE} DIRECTORY)
        string(REPLACE "/" "_" TEST_TARGET "${TEST_DIR}_${TEST_NAME}")

        if(TARGET ${TEST_TARGET})
            target_compile_options(${TEST_TARGET} PRIVATE --coverage)
            target_link_options(${TEST_TARGET} PRIVATE --coverage)
        endif()
    endforeach()
endif()

# Custom test targets
add_custom_target(run_integration_scenarios
    COMMAND ${CMAKE_CTEST_COMMAND} -L scenarios -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running scenario integration tests"
)

add_custom_target(run_integration_performance
    COMMAND ${CMAKE_CTEST_COMMAND} -L performance -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running performance integration tests"
)

add_custom_target(run_integration_failures
    COMMAND ${CMAKE_CTEST_COMMAND} -L failures -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running failure handling integration tests"
)

add_custom_target(run_all_integration
    COMMAND ${CMAKE_CTEST_COMMAND} -L integration -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all integration tests"
)

# Test configuration summary
message(STATUS "Container System Integration Tests Configuration:")
message(STATUS "  GTest found: ${GTest_FOUND}")
message(STATUS "  Test count: ${INTEGRATION_TEST_SOURCES}")
message(STATUS "  Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "  Test framework directory: ${CMAKE_CURRENT_SOURCE_DIR}/framework")
