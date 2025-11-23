# WORKFLOW-001: Add Windows Build Support

## Metadata
- **ID**: WORKFLOW-001
- **Priority**: MEDIUM
- **Estimated Time**: 2 days
- **Dependencies**: None
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [WORKFLOW_IMPROVEMENTS.md](../../.github/workflows/WORKFLOW_IMPROVEMENTS.md)

---

## Overview

### What are we trying to change?

Add Windows platform build to the GitHub Actions CI/CD pipeline to complete cross-platform support.

**Current State**:
- Ubuntu (GCC) build supported
- Ubuntu (Clang) build supported
- macOS (Apple Clang) build supported
- Windows support **not available**

**Goals**:
- MSYS2/MinGW-w64 build support
- Visual Studio 2022 build support
- Windows-specific test execution

---

## Changes

### How are we going to implement this?

#### 1. MSYS2 Build Workflow

**`.github/workflows/build-windows-msys2.yaml`**:
```yaml
name: Windows MSYS2 Build

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:

jobs:
  build-msys2:
    runs-on: windows-latest
    defaults:
      run:
        shell: msys2 {0}

    steps:
      - uses: actions/checkout@v4

      - name: Setup MSYS2
        uses: msys2/setup-msys2@v2
        with:
          msystem: UCRT64
          update: true
          install: >-
            mingw-w64-ucrt-x86_64-toolchain
            mingw-w64-ucrt-x86_64-cmake
            mingw-w64-ucrt-x86_64-ninja
            mingw-w64-ucrt-x86_64-fmt
            mingw-w64-ucrt-x86_64-nlohmann-json
            mingw-w64-ucrt-x86_64-pugixml
            mingw-w64-ucrt-x86_64-gtest

      - name: Configure
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DENABLE_SIMD=ON \
            -DUSE_UNIT_TEST=ON \
            -DBUILD_CONTAINER_EXAMPLES=ON

      - name: Build
        run: cmake --build build -j$(nproc)

      - name: Test
        run: |
          cd build
          ctest --output-on-failure

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: windows-msys2-artifacts
          path: |
            build/bin/
            build/lib/
          retention-days: 30
```

#### 2. Visual Studio Build Workflow

**`.github/workflows/build-windows-msvc.yaml`**:
```yaml
name: Windows MSVC Build

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:

jobs:
  build-msvc:
    runs-on: windows-latest

    steps:
      - uses: actions/checkout@v4

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: '1de2026f28ead93ff1773e6e680387643e914ea1'

      - name: Configure CMake
        run: |
          cmake -B build -G "Visual Studio 17 2022" -A x64 `
            -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
            -DCMAKE_BUILD_TYPE=Release `
            -DENABLE_SIMD=ON `
            -DUSE_UNIT_TEST=ON `
            -DBUILD_CONTAINER_EXAMPLES=ON

      - name: Build
        run: cmake --build build --config Release -j

      - name: Test
        run: |
          cd build
          ctest -C Release --output-on-failure

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: windows-msvc-artifacts
          path: |
            build/bin/Release/
            build/lib/Release/
          retention-days: 30
```

#### 3. CMakeLists.txt Windows Compatibility Modifications

```cmake
# Windows-specific settings
if(WIN32)
    # MSVC warning settings
    if(MSVC)
        add_compile_options(/W4 /WX-)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-DNOMINMAX)  # Prevent min/max macro conflicts
    endif()

    # MinGW settings
    if(MINGW)
        add_compile_options(-Wall -Wextra)
    endif()

    # Windows-specific libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
```

#### 4. Platform-Specific Code Modifications

```cpp
// simd_processor.h - Windows SIMD support
#if defined(_WIN32)
    #if defined(_M_X64) || defined(_M_IX86)
        #include <intrin.h>
        #define CONTAINER_SIMD_AVX2
    #endif
#endif
```

---

## Test Plan

### How will we test this?

#### 1. Local Windows Build Test

```powershell
# Visual Studio build
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cd build && ctest -C Release --output-on-failure
```

#### 2. CI Workflow Test

- Create a PR to trigger the workflow
- Verify build logs
- Verify test results

#### 3. Binary Compatibility Test

```cpp
TEST(WindowsCompatibility, BinaryCompatibility) {
    // Verify that data created on Windows can be read on Linux/macOS
    // and vice versa
}
```

#### Success Criteria
- [ ] MSYS2 build successful
- [ ] MSVC build successful
- [ ] All tests pass
- [ ] Binary compatibility maintained

---

## Technical Considerations

### Windows-Specific Issues

1. **long type size**: Windows is 32-bit, Unix is 64-bit
   - Already resolved with `long_value` 32-bit policy

2. **File paths**: Backslash vs forward slash
   - CMake handles this automatically

3. **DLL Export/Import**:
```cpp
#ifdef _WIN32
    #ifdef CONTAINER_EXPORTS
        #define CONTAINER_API __declspec(dllexport)
    #else
        #define CONTAINER_API __declspec(dllimport)
    #endif
#else
    #define CONTAINER_API
#endif
```

---

## Checklist

- [ ] Create build-windows-msys2.yaml
- [ ] Create build-windows-msvc.yaml
- [ ] Add Windows compatibility to CMakeLists.txt
- [ ] Verify SIMD Windows support
- [ ] Local Windows build test
- [ ] CI workflow test
- [ ] Binary compatibility test
- [ ] Update Build Matrix documentation

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
