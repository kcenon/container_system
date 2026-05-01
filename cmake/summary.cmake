# summary.cmake - end-of-configure status summary
#
# Prints a concise summary of the container_system configuration. Reads
# COMPILER_SUPPORTS_AVX2 / COMPILER_SUPPORTS_SSE42 which are exported from
# cmake/compile_options.cmake's apply_container_simd_definitions() helper.

message(STATUS "Container library configured:")
message(STATUS "  Module Directory: ${CMAKE_CURRENT_SOURCE_DIR}/container")
message(STATUS "  Dependencies: utilities, std::format (C++20, fallback available)")
message(STATUS "  Features:")
message(STATUS "    - Thread-safe operations: ON")
message(STATUS "    - Variant-based types: ON")
message(STATUS "    - Build samples: ${BUILD_CONTAINER_SAMPLES}")
message(STATUS "    - C++20 modules: ${CONTAINER_BUILD_MODULES}")
if(COMPILER_SUPPORTS_AVX2)
    message(STATUS "    - AVX2 SIMD: ON")
elseif(COMPILER_SUPPORTS_SSE42)
    message(STATUS "    - SSE4.2 SIMD: ON")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "    - ARM NEON SIMD: ON")
else()
    message(STATUS "    - SIMD: OFF (scalar fallback)")
endif()
