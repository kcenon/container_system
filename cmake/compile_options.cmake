# compile_options.cmake - language standards, output dirs, debug flags
#
# Sets project-wide compiler settings that must be in place BEFORE targets are
# created. Per-target warning/SIMD/platform flags are applied later by
# cmake/targets.cmake via apply_container_compile_options().

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

# Platform-specific top-level definitions (applied via add_definitions so they
# affect every target created afterwards)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
elseif(APPLE)
    add_definitions(-DAPPLE_PLATFORM)
endif()

#[[
apply_container_compile_options(<target>)

Applies the container_system warning policy and platform-specific compile
flags to the named target. Split into a function so that the same set of
flags can be reused for both the static library target and the optional
C++20 module target.
]]
function(apply_container_compile_options target_name)
    # Suppress warnings inherited from parent project
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wno-sign-conversion
            -Wno-shorten-64-to-32
            -Wno-float-equal
            -Wno-implicit-float-conversion
            -Wno-unreachable-code-loop-increment
        )
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE
            /wd4244  # conversion from 'type1' to 'type2', possible loss of data
            /wd4267  # conversion from 'size_t' to 'type', possible loss of data
            /wd4305  # truncation from 'type1' to 'type2'
            /wd4365  # conversion from 'type1' to 'type2', signed/unsigned mismatch
            /wd4668  # 'symbol' is not defined as a preprocessor macro
        )
    endif()

    # Standard warning set
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
            -Wno-unused-variable
        )
        # Clang-specific warnings
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target_name} PRIVATE
                -Wno-unused-lambda-capture
            )
        endif()
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /WX-
            /wd4100  # unreferenced formal parameter
            /wd4996  # deprecated functions
        )
    endif()

    # Platform-specific definitions
    if(WIN32)
        target_compile_definitions(${target_name} PRIVATE
            _WIN32_WINNT=0x0601  # Windows 7 or later
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()
endfunction()

#[[
apply_container_simd_definitions(<target>)

Detects compiler SIMD support and adds the matching HAS_AVX2 / HAS_SSE42 /
HAS_ARM_NEON private compile definitions. AVX2/SSE2 functions opt in via
__attribute__((target("avx2"))) so no global -mavx2 / -msse4.2 flags are
added — this matches the legacy behaviour described in the original
CMakeLists.txt comments.
]]
function(apply_container_simd_definitions target_name)
    include(CheckCXXCompilerFlag)

    # AVX2 support
    if(NOT APPLE)
        check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
        if(COMPILER_SUPPORTS_AVX2)
            target_compile_definitions(${target_name} PRIVATE HAS_AVX2)
        endif()
    endif()

    # SSE support (x86/x64 only)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
        check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
        if(COMPILER_SUPPORTS_SSE42)
            target_compile_definitions(${target_name} PRIVATE HAS_SSE42)
        endif()
    endif()

    # ARM NEON support
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        target_compile_definitions(${target_name} PRIVATE HAS_ARM_NEON)
    endif()

    # Re-export the detection results to the parent scope so the summary
    # module can read them (PARENT_SCOPE) without re-running the checks.
    set(COMPILER_SUPPORTS_AVX2 "${COMPILER_SUPPORTS_AVX2}" PARENT_SCOPE)
    set(COMPILER_SUPPORTS_SSE42 "${COMPILER_SUPPORTS_SSE42}" PARENT_SCOPE)
endfunction()
