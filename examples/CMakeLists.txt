##################################################
# Container System Examples CMakeLists.txt
##################################################

cmake_minimum_required(VERSION 3.16)

# Example executables
set(EXAMPLE_PROGRAMS
    basic_container_example
    messaging_integration_example
    advanced_container_example
    real_world_scenarios
)

# Add async coroutine example if coroutines are enabled
if(CONTAINER_ENABLE_COROUTINES)
    list(APPEND EXAMPLE_PROGRAMS async_coroutine_example)
    message(STATUS "Container Example: async_coroutine_example enabled")
endif()

# Check for Asio availability
find_package(asio CONFIG QUIET)
if(asio_FOUND)
    list(APPEND EXAMPLE_PROGRAMS asio_integration_example)
    message(STATUS "Container Example: asio_integration_example enabled (Asio found)")
endif()

# TODO: Result pattern example requires container API updates
# if(BUILD_WITH_COMMON_SYSTEM)
#     list(APPEND EXAMPLE_PROGRAMS result_pattern_example)
#     message(STATUS "Result pattern example enabled (BUILD_WITH_COMMON_SYSTEM=ON)")
# endif()

# Build each example program
foreach(EXAMPLE ${EXAMPLE_PROGRAMS})
    add_executable(${EXAMPLE} ${EXAMPLE}.cpp)

    # Link with container library
    target_link_libraries(${EXAMPLE}
        PRIVATE
            container_system
            Threads::Threads
    )

    # Set C++20 standard
    set_target_properties(${EXAMPLE} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Include directories
    target_include_directories(${EXAMPLE} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # Add Asio include directories for asio_integration_example
    if(${EXAMPLE} STREQUAL "asio_integration_example" AND asio_FOUND)
        target_link_libraries(${EXAMPLE} PRIVATE asio::asio)
    endif()

    # Add coroutine flags for async examples
    if(${EXAMPLE} MATCHES "async_.*_example" AND CONTAINER_ENABLE_COROUTINES)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${EXAMPLE} PRIVATE -fcoroutines)
        endif()
        target_compile_definitions(${EXAMPLE} PRIVATE CONTAINER_HAS_COROUTINES=1)
    endif()

    # Add compile definitions based on enabled features
    if(ENABLE_MESSAGING_FEATURES)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_MESSAGING_FEATURES)
    endif()

    if(ENABLE_PERFORMANCE_METRICS)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_PERFORMANCE_METRICS)
    endif()

    if(ENABLE_EXTERNAL_INTEGRATION)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_EXTERNAL_INTEGRATION)
    endif()

    # Debug/Release configurations
    target_compile_definitions(${EXAMPLE} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(${EXAMPLE} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()

    # Install example executables
    install(TARGETS ${EXAMPLE}
        RUNTIME DESTINATION bin/examples
    )

    message(STATUS "Container Example: ${EXAMPLE} configured")
endforeach()

# Create example README
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_BINARY_DIR}/README.md
    COPYONLY
)

install(FILES README.md
    DESTINATION share/examples
)