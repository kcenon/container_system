# BSD 3-Clause License
#
# Copyright (c) 2024, kcenon
# All rights reserved.
#
# Container System gRPC Integration - Isolated Build Configuration
# This CMakeLists.txt is completely separate from the main build.
# No modifications to existing code are required.

cmake_minimum_required(VERSION 3.16)
project(container_grpc VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# Options
# =============================================================================

option(CONTAINER_GRPC_BUILD_TESTS "Build gRPC integration tests" ON)
option(CONTAINER_GRPC_BUILD_EXAMPLES "Build gRPC examples" ON)

# =============================================================================
# C++ Standard
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Dependencies
# =============================================================================

# Find Protobuf
find_package(Protobuf REQUIRED)
message(STATUS "Found Protobuf: ${Protobuf_VERSION}")

# Find gRPC
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Found gRPC: ${gRPC_VERSION}")

# Find Threads
find_package(Threads REQUIRED)

# =============================================================================
# Proto Generation
# =============================================================================

# Get the protoc and grpc_cpp_plugin executables
get_target_property(PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin IMPORTED_LOCATION)

# Define proto file
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/container_service.proto")

# Output directory for generated files
set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# Generated file names
set(PROTO_SRCS "${PROTO_OUTPUT_DIR}/container_service.pb.cc")
set(PROTO_HDRS "${PROTO_OUTPUT_DIR}/container_service.pb.h")
set(GRPC_SRCS "${PROTO_OUTPUT_DIR}/container_service.grpc.pb.cc")
set(GRPC_HDRS "${PROTO_OUTPUT_DIR}/container_service.grpc.pb.h")

# Generate protobuf files
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
        --cpp_out=${PROTO_OUTPUT_DIR}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf sources"
    VERBATIM
)

# Generate gRPC files
add_custom_command(
    OUTPUT ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
        --grpc_out=${PROTO_OUTPUT_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC sources"
    VERBATIM
)

# =============================================================================
# Library: container_grpc_proto
# =============================================================================

add_library(container_grpc_proto STATIC
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

target_include_directories(container_grpc_proto PUBLIC
    ${PROTO_OUTPUT_DIR}
)

target_link_libraries(container_grpc_proto PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

# Suppress warnings in generated code
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(container_grpc_proto PRIVATE
        -Wno-unused-parameter
        -Wno-deprecated-declarations
    )
endif()

# =============================================================================
# Library: container_grpc_adapter
# =============================================================================

# Adapter sources
set(ADAPTER_SOURCES
    adapters/container_adapter.cpp
)

add_library(container_grpc_adapter STATIC
    ${ADAPTER_SOURCES}
)

target_include_directories(container_grpc_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # Access existing container headers (read-only)
    ${PROTO_OUTPUT_DIR}
)

target_link_libraries(container_grpc_adapter PUBLIC
    container_grpc_proto
)

# =============================================================================
# Library: container_grpc (Full gRPC library)
# =============================================================================

# Service implementation sources
set(SERVICE_SOURCES
    server/grpc_server.cpp
    server/service_impl.cpp
    client/grpc_client.cpp
)

add_library(container_grpc STATIC
    ${SERVICE_SOURCES}
)

target_include_directories(container_grpc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${PROTO_OUTPUT_DIR}
)

target_link_libraries(container_grpc PUBLIC
    container_grpc_adapter
    container_grpc_proto
    gRPC::grpc++
    Threads::Threads
)

# =============================================================================
# Examples (Optional)
# =============================================================================

if(CONTAINER_GRPC_BUILD_EXAMPLES)
    # Examples will be added in Week 3
    # add_executable(grpc_server_example examples/server_example.cpp)
    # target_link_libraries(grpc_server_example PRIVATE container_grpc)

    # add_executable(grpc_client_example examples/client_example.cpp)
    # target_link_libraries(grpc_client_example PRIVATE container_grpc)
endif()

# =============================================================================
# Tests (Optional)
# =============================================================================

if(CONTAINER_GRPC_BUILD_TESTS)
    # Tests will be added in Week 3
    # find_package(GTest REQUIRED)
    # add_executable(grpc_adapter_test tests/adapter_test.cpp)
    # target_link_libraries(grpc_adapter_test PRIVATE
    #     container_grpc
    #     GTest::gtest
    #     GTest::gtest_main
    # )
    # add_test(NAME grpc_adapter_test COMMAND grpc_adapter_test)
endif()

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "Container gRPC Integration Configuration:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Protobuf:       ${Protobuf_VERSION}")
message(STATUS "  gRPC:           ${gRPC_VERSION}")
message(STATUS "  Build Tests:    ${CONTAINER_GRPC_BUILD_TESTS}")
message(STATUS "  Build Examples: ${CONTAINER_GRPC_BUILD_EXAMPLES}")
message(STATUS "")
