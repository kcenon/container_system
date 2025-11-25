# WORKFLOW-002: ARM64 Linux Cross-Compilation

## Metadata
- **ID**: WORKFLOW-002
- **Priority**: LOW
- **Estimated Time**: 1 day
- **Dependencies**: None
- **Status**: DONE
- **Assignee**: TBD
- **Related Documents**: [WORKFLOW_IMPROVEMENTS.md](../../.github/workflows/WORKFLOW_IMPROVEMENTS.md)

---

## Overview

### What are we trying to change?

Enable cross-compilation for ARM64 Linux targets from x86_64 hosts to support deployment on ARM servers and embedded environments.

**Current State**:
- macOS ARM64 (native) supported
- Linux ARM64 (native) not supported
- Linux ARM64 (cross-compilation) not supported

**Goals**:
- Build ARM64 Linux binaries on x86_64 Ubuntu
- Run tests via QEMU
- Support ARM64 NEON SIMD

---

## Changes

### How are we going to implement this?

#### 1. ARM64 Cross-Compilation Workflow

**`.github/workflows/build-linux-arm64.yaml`**:
```yaml
name: Linux ARM64 Cross-Compile

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:

jobs:
  build-arm64:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Install Cross-Compilation Tools
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            gcc-aarch64-linux-gnu \
            g++-aarch64-linux-gnu \
            qemu-user-static \
            qemu-user

      - name: Setup vcpkg for ARM64
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          ./vcpkg/bootstrap-vcpkg.sh
          export VCPKG_ROOT=$(pwd)/vcpkg
          echo "VCPKG_ROOT=$VCPKG_ROOT" >> $GITHUB_ENV

      - name: Install ARM64 Dependencies
        run: |
          $VCPKG_ROOT/vcpkg install --triplet=arm64-linux \
            fmt nlohmann-json pugixml gtest

      - name: Configure CMake
        run: |
          cmake -B build \
            -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
            -DVCPKG_TARGET_TRIPLET=arm64-linux \
            -DCMAKE_SYSTEM_NAME=Linux \
            -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
            -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
            -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
            -DCMAKE_BUILD_TYPE=Release \
            -DENABLE_SIMD=ON \
            -DUSE_UNIT_TEST=ON

      - name: Build
        run: cmake --build build -j$(nproc)

      - name: Test with QEMU
        run: |
          cd build
          # Run ARM64 binary with QEMU
          qemu-aarch64-static -L /usr/aarch64-linux-gnu ./bin/container_unit_tests

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: linux-arm64-artifacts
          path: |
            build/bin/
            build/lib/
          retention-days: 30
```

#### 2. ARM64 Toolchain File

**`cmake/toolchain-arm64-linux.cmake`**:
```cmake
# ARM64 Linux Cross-Compilation Toolchain

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross-compiler settings
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Sysroot settings (if needed)
# set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)

# Find mode settings
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARM64 NEON flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+simd")
```

#### 3. SIMD Cross-Compilation Support

```cpp
// simd_processor.h - ARM64 cross-compilation detection
#if defined(__aarch64__) || defined(_M_ARM64)
    // ARM64 native or cross-compilation
    #include <arm_neon.h>
    #define CONTAINER_SIMD_NEON
#endif
```

---

## Test Plan

### How will we test this?

#### 1. Local Cross-Compilation Test

```bash
# Install cross-compiler
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Build
cmake -B build-arm64 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm64-linux.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-arm64

# Verify binary
file build-arm64/bin/container_unit_tests
# Output: ELF 64-bit LSB executable, ARM aarch64
```

#### 2. QEMU Test

```bash
# Install QEMU
sudo apt-get install qemu-user-static

# Run tests
qemu-aarch64-static -L /usr/aarch64-linux-gnu \
  build-arm64/bin/container_unit_tests
```

#### 3. Actual ARM64 Hardware Test (Optional)

- AWS Graviton instance
- Raspberry Pi 4
- ARM64 server

#### Success Criteria
- [ ] Cross-compilation successful
- [ ] QEMU tests pass
- [ ] NEON SIMD activation confirmed
- [ ] Actual hardware test (optional)

---

## Technical Considerations

### ARM64 NEON vs x86 AVX2

| Feature | ARM64 NEON | x86 AVX2 |
|---------|------------|----------|
| Register width | 128-bit | 256-bit |
| Number of registers | 32 | 16 |
| Throughput | Good | Very good |
| Power efficiency | Very good | Average |

### Cross-Compilation Considerations

1. **Endianness**: ARM64 is Little-endian by default (same as x86)
2. **Data alignment**: ARM can be more sensitive to alignment
3. **Runtime libraries**: Static linking recommended

---

## Checklist

- [x] Create build-linux-arm64.yaml
- [x] Create toolchain-arm64-linux.cmake
- [x] Verify SIMD cross-compilation support
- [x] Local cross-compilation test
- [x] QEMU test
- [x] CI workflow test
- [x] Update documentation

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-25
**Completed**: 2025-11-25
