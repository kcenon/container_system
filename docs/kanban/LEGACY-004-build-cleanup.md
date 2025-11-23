# LEGACY-004: Build System Cleanup

## Metadata
- **ID**: LEGACY-004
- **Priority**: LOW
- **Estimated Duration**: 0.5 days
- **Dependencies**: LEGACY-001, LEGACY-002, LEGACY-003
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [LEGACY_REMOVAL_PLAN.md](../advanced/LEGACY_REMOVAL_PLAN.md)

---

## Overview

### What are we changing?

Clean up and optimize CMakeLists.txt and build scripts after legacy system removal.

**Cleanup targets**:
- Legacy file references in CMakeLists.txt
- Unused build options
- vcpkg dependencies (if needed)
- Include paths

---

## Changes

### How will we implement this?

#### 1. CMakeLists.txt Cleanup

**Source list to remove**:
```cmake
# Before (remove)
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/container.cpp
    # ... legacy sources
)

set(VALUE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/values/bool_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/string_value.cpp
    # ... legacy sources
)

# After (cleaned)
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.cpp
)

set(INTERNAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_v2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.cpp
)
```

**Header list to remove**:
```cmake
# Remove
set(VALUE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/values/bool_value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/values/numeric_value.h
    # ...
)
```

#### 2. Include Path Cleanup

```cmake
# Before
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/values      # Remove
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_CURRENT_SOURCE_DIR}/include/container
)

# After
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core        # value_types only
    ${CMAKE_CURRENT_SOURCE_DIR}/internal    # Main implementation
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_CURRENT_SOURCE_DIR}/include/container
)
```

#### 3. Build Option Cleanup

```cmake
# Options to remove
option(USE_LEGACY_VALUE_SYSTEM "Use legacy polymorphic value system" OFF)  # Remove
option(ENABLE_VALUE_POOL "Enable memory pooling for values" OFF)  # Remove

# Options to keep
option(ENABLE_MESSAGING_FEATURES "Enable messaging optimizations" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(USE_THREAD_SAFE_OPERATIONS "Enable thread-safe operations" ON)
```

#### 4. vcpkg.json Review

```json
{
    "name": "container-system",
    "version": "2.0.0",
    "dependencies": [
        "fmt",
        "nlohmann-json",
        "pugixml"
        // Review dependencies to remove if needed
    ],
    "features": {
        "tests": {
            "dependencies": [
                "gtest"
            ]
        }
    }
}
```

#### 5. Documentation Update

**README.md updates**:
- Remove legacy API examples
- Replace with new API examples
- Update build instructions

---

## Test Plan

### How will we test this?

#### 1. Clean Build Test

```bash
# Full clean build after deleting build directory
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### 2. All Platform Builds

```bash
# Linux (GCC)
cmake -B build-gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build-gcc

# Linux (Clang)
cmake -B build-clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-clang

# macOS
cmake -B build-macos
cmake --build build-macos
```

#### 3. Test Suite Execution

```bash
cd build && ctest --output-on-failure -j$(nproc)
```

#### 4. Build Time Comparison

```bash
# Measure build time before cleanup
time cmake --build build-before --clean-first

# Measure build time after cleanup
time cmake --build build-after --clean-first
```

#### Success Criteria
- [ ] All platform builds succeed
- [ ] 100% test pass
- [ ] Build time reduced (expected 10%+)
- [ ] Binary size reduced

---

## Metrics

### Expected Changes

| Item | Before Cleanup | After Cleanup | Change |
|------|----------------|---------------|--------|
| Source file count | ~58 | ~40 | -31% |
| Header file count | ~28 | ~20 | -29% |
| CMakeLists.txt LOC | ~600 | ~450 | -25% |
| Build time | TBD | TBD | Expected -10% |
| Library size | TBD | TBD | Expected -15% |

---

## Checklist

- [ ] Remove legacy sources from CMakeLists.txt
- [ ] Remove legacy headers from CMakeLists.txt
- [ ] Clean up include paths
- [ ] Clean up build options
- [ ] Review vcpkg.json
- [ ] Clean build test
- [ ] Verify all platform builds
- [ ] Test suite pass
- [ ] Measure build time
- [ ] Update README.md

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
