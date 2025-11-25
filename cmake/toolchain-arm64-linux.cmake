# ARM64 Linux Cross-Compilation Toolchain
# Usage: cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm64-linux.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross-compiler settings
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Linker settings
set(CMAKE_AR aarch64-linux-gnu-ar)
set(CMAKE_RANLIB aarch64-linux-gnu-ranlib)
set(CMAKE_STRIP aarch64-linux-gnu-strip)

# Sysroot settings (for finding ARM64 libraries)
# set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# Search settings for cross-compilation
# NEVER: Use host tools for programs (cmake, etc.)
# ONLY: Search only in target sysroot for libraries/includes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARM64 compiler flags
# -march=armv8-a: Base ARMv8-A architecture
# +simd: Enable SIMD (NEON) instructions
# +crc: Enable CRC32 instructions
set(CMAKE_C_FLAGS_INIT "-march=armv8-a+simd+crc")
set(CMAKE_CXX_FLAGS_INIT "-march=armv8-a+simd+crc")

# Enable position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define that we're cross-compiling for ARM64
add_definitions(-DARM64_CROSS_COMPILE)

# Indicate NEON support for SIMD code
add_definitions(-DHAS_ARM_NEON)
