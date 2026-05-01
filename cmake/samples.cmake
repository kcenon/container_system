# samples.cmake - samples, examples, and benchmarks subdirectories
#
# Conditionally includes the samples/, examples/, and benchmarks/
# subdirectories. All three are gated on NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE
# so that downstream consumers including container_system as a submodule do
# not pull in extra targets unexpectedly.

if(NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    if(BUILD_CONTAINER_SAMPLES)
        add_subdirectory(samples)
        message(STATUS "Container samples will be built")
    endif()

    # Build examples (migrated to variant-based API - REACT-004 complete)
    if(BUILD_CONTAINER_EXAMPLES)
        add_subdirectory(examples)
        message(STATUS "Container examples will be built")
    endif()
else()
    message(STATUS "Container samples and examples disabled (submodule mode)")
endif()

# Benchmarks (migrated to variant-based API)
if(CONTAINER_BUILD_BENCHMARKS AND NOT BUILD_CONTAINERSYSTEM_AS_SUBMODULE)
    add_subdirectory(benchmarks)
    message(STATUS "Container benchmarks will be built")
endif()
