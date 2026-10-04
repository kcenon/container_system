cmake_minimum_required(VERSION 3.20)

##################################################
# Container System CMakeLists.txt (thin orchestrator)
#
# Container-specific optimizations for common_system.
#
#   - fast_parser.h: High-performance parsing optimizations
#   - Type-safe, serializable data containers with SIMD acceleration
#   - Dependencies managed via CMake FetchContent
#
# All concerns live in per-area modules under cmake/. This top-level file
# is intentionally short — it only declares the project and includes the
# modules in dependency order. See cmake/README.md for a per-module map.
#
# Decomposition tracked under issue #535 (part of EPIC #531).
# See SCOPE.md for project-wide boundaries and responsibilities.
##################################################

# Project definition
project(container_system
    VERSION 1.0.0
    DESCRIPTION "Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration"
    LANGUAGES CXX
)

# Order matters:
#   options        -> declares feature toggles consumed by every later module
#   compile_options-> language standards, output dirs, warning helpers
#   dependencies   -> Threads, common_system imported targets
#   sources        -> file lists (no targets created here)
#   targets        -> add_library(container_system) + flags + defs + links
#   modules        -> optional C++20 module library target
#   test           -> unit / integration / fuzz targets
#   documentation  -> Doxygen target
#   samples        -> samples / examples / benchmarks subdirectories
#   install        -> install rules and package config generation
#   summary        -> end-of-configure status print
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compile_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/test.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/documentation.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/samples.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/summary.cmake)
