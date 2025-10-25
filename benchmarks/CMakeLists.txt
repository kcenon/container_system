# Container System Benchmarks
# Phase 0, Task 0.2: Baseline Performance Benchmarking

cmake_minimum_required(VERSION 3.20)

find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(BENCHMARK benchmark)
    endif()
endif()

if(benchmark_FOUND OR BENCHMARK_FOUND)
    message(STATUS "Google Benchmark found. Building container_system benchmarks...")

    add_executable(container_benchmarks
        container_operations_bench.cpp
        serialization_bench.cpp
        value_operations_bench.cpp
        memory_efficiency_bench.cpp
        main_bench.cpp
    )

    target_link_libraries(container_benchmarks
        PRIVATE
            container_system
            benchmark::benchmark
            benchmark::benchmark_main
    )

    # Add include directories for proper header resolution
    target_include_directories(container_benchmarks
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_compile_features(container_benchmarks PRIVATE cxx_std_20)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(container_benchmarks PRIVATE
            -Wall -Wextra -Wpedantic -O3 -DNDEBUG)
    elseif(MSVC)
        target_compile_options(container_benchmarks PRIVATE /W4 /O2 /DNDEBUG)
    endif()

    add_custom_target(run_container_benchmarks
        COMMAND container_benchmarks --benchmark_format=json --benchmark_out=results.json
        COMMAND container_benchmarks
        DEPENDS container_benchmarks
        COMMENT "Running container_system benchmarks"
    )

    install(TARGETS container_benchmarks RUNTIME DESTINATION bin/benchmarks)
else()
    message(WARNING "Google Benchmark not found. Skipping benchmarks.")
endif()
