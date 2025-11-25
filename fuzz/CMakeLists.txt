# Fuzzing targets for Container System
# Requires Clang with -fsanitize=fuzzer support

# Check if we're using Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(BUILD_FUZZING "Build fuzz targets" OFF)

    if(BUILD_FUZZING)
        message(STATUS "Building fuzz targets with libFuzzer")

        # Add fuzzer and sanitizer flags
        set(FUZZ_COMPILE_FLAGS "-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer")
        set(FUZZ_LINK_FLAGS "-fsanitize=fuzzer,address,undefined")

        # Fuzz target for value deserialization
        add_executable(fuzz_deserialize fuzz_deserialize.cpp)
        target_link_libraries(fuzz_deserialize PRIVATE container_system)
        target_include_directories(fuzz_deserialize PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/internal
        )
        set_target_properties(fuzz_deserialize PROPERTIES
            COMPILE_FLAGS "${FUZZ_COMPILE_FLAGS}"
            LINK_FLAGS "${FUZZ_LINK_FLAGS}"
        )

        # Fuzz target for container deserialization
        add_executable(fuzz_container_deserialize fuzz_container_deserialize.cpp)
        target_link_libraries(fuzz_container_deserialize PRIVATE container_system)
        target_include_directories(fuzz_container_deserialize PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/internal
        )
        set_target_properties(fuzz_container_deserialize PROPERTIES
            COMPILE_FLAGS "${FUZZ_COMPILE_FLAGS}"
            LINK_FLAGS "${FUZZ_LINK_FLAGS}"
        )

        # Create corpus directories
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/corpus/deserialize)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/corpus/container)

        # Copy seed corpus if it exists
        if(EXISTS ${CMAKE_SOURCE_DIR}/fuzz/corpus)
            file(COPY ${CMAKE_SOURCE_DIR}/fuzz/corpus DESTINATION ${CMAKE_BINARY_DIR})
        endif()
    endif()
else()
    message(STATUS "Fuzzing targets require Clang compiler - skipping")
endif()
