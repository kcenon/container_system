# WORKFLOW-003: Fuzzing Automation

## Metadata
- **ID**: WORKFLOW-003
- **Priority**: MEDIUM
- **Estimated Time**: 2 days
- **Dependencies**: None
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [WORKFLOW_IMPROVEMENTS.md](../../.github/workflows/WORKFLOW_IMPROVEMENTS.md)

---

## Overview

### What are we trying to change?

Add automated fuzz testing using OSS-Fuzz or libFuzzer to detect security vulnerabilities in serialization/deserialization code.

**Current State**:
- Only static analysis (cppcheck, clang-tidy) performed
- No fuzz testing

**Goals**:
- Fuzz Deserialize functions
- Verify memory safety
- CI-integrated fuzzing

---

## Changes

### How are we going to implement this?

#### 1. Write Fuzz Targets

**`fuzz/fuzz_deserialize.cpp`**:
```cpp
#include <cstdint>
#include <cstddef>
#include <vector>

#include "internal/variant_value_v2.h"
#include "internal/thread_safe_container.h"

using namespace container_module;

// libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Attempt to deserialize variant_value_v2 from input data
    std::vector<uint8_t> input(data, data + size);

    try {
        // Single value deserialize
        auto result = variant_value_v2::deserialize(input);
        if (result.has_value()) {
            // If successfully deserialized, serialize again to verify consistency
            auto serialized = result->serialize();
            (void)serialized;  // Suppress unused warning
        }
    } catch (...) {
        // Exceptions are allowed (invalid input)
    }

    return 0;
}
```

**`fuzz/fuzz_container_deserialize.cpp`**:
```cpp
#include <cstdint>
#include <cstddef>
#include <vector>

#include "internal/thread_safe_container.h"

using namespace container_module;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::vector<uint8_t> input(data, data + size);

    try {
        auto container = thread_safe_container::deserialize(input);
        if (container) {
            // Round-trip test
            auto serialized = container->serialize_array();
            (void)serialized;
        }
    } catch (...) {
        // Exceptions are allowed
    }

    return 0;
}
```

#### 2. CMake Fuzz Target Configuration

**`fuzz/CMakeLists.txt`**:
```cmake
# Fuzzing requires Clang with -fsanitize=fuzzer
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(BUILD_FUZZING "Build fuzz targets" OFF)

    if(BUILD_FUZZING)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer,address,undefined")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=fuzzer,address,undefined")

        # Fuzz targets
        add_executable(fuzz_deserialize fuzz_deserialize.cpp)
        target_link_libraries(fuzz_deserialize PRIVATE container_system)

        add_executable(fuzz_container_deserialize fuzz_container_deserialize.cpp)
        target_link_libraries(fuzz_container_deserialize PRIVATE container_system)
    endif()
endif()
```

#### 3. GitHub Actions Fuzzing Workflow

**`.github/workflows/fuzzing.yaml`**:
```yaml
name: Fuzzing Tests

on:
  schedule:
    - cron: '0 2 * * 1'  # Every Monday at 02:00 UTC
  workflow_dispatch:

jobs:
  fuzz:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Install Clang
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-17 llvm-17

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11

      - name: Configure with Fuzzing
        run: |
          CC=clang-17 CXX=clang++-17 cmake -B build \
            -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
            -DCMAKE_BUILD_TYPE=Debug \
            -DBUILD_FUZZING=ON

      - name: Build Fuzz Targets
        run: cmake --build build --target fuzz_deserialize fuzz_container_deserialize

      - name: Create Seed Corpus
        run: |
          mkdir -p corpus/deserialize
          mkdir -p corpus/container

          # Generate valid input samples
          echo -n "valid_seed" | xxd -r -p > corpus/deserialize/seed1
          # ... add more seeds

      - name: Run Fuzzing (5 minutes)
        run: |
          timeout 300 ./build/fuzz/fuzz_deserialize corpus/deserialize || true
          timeout 300 ./build/fuzz/fuzz_container_deserialize corpus/container || true

      - name: Check for Crashes
        run: |
          if ls crash-* 1> /dev/null 2>&1; then
            echo "Crashes found!"
            ls -la crash-*
            exit 1
          fi
          echo "No crashes found"

      - name: Upload Corpus
        uses: actions/upload-artifact@v4
        with:
          name: fuzz-corpus
          path: corpus/
          retention-days: 90

      - name: Upload Crashes (if any)
        if: failure()
        uses: actions/upload-artifact@v4
        with:
          name: fuzz-crashes
          path: crash-*
```

#### 4. Seed Corpus Generation Script

**`fuzz/generate_corpus.py`**:
```python
#!/usr/bin/env python3
"""Generate seed corpus for fuzzing."""

import os
import struct

def generate_valid_int_value():
    """Generate a valid serialized int value."""
    name = b"test"
    name_len = struct.pack("<I", len(name))
    type_byte = bytes([4])  # int_value
    value = struct.pack("<i", 12345)
    return name_len + name + type_byte + value

def main():
    os.makedirs("corpus/deserialize", exist_ok=True)

    # Valid inputs
    with open("corpus/deserialize/valid_int", "wb") as f:
        f.write(generate_valid_int_value())

    # Edge cases
    with open("corpus/deserialize/empty", "wb") as f:
        f.write(b"")

    with open("corpus/deserialize/minimal", "wb") as f:
        f.write(b"\x00\x00\x00\x00")

    print("Corpus generated successfully")

if __name__ == "__main__":
    main()
```

---

## Test Plan

### How will we test this?

#### 1. Local Fuzzing Test

```bash
# Build with Clang
CC=clang CXX=clang++ cmake -B build-fuzz \
  -DBUILD_FUZZING=ON \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build-fuzz

# Run fuzzing (1 minute)
./build-fuzz/fuzz/fuzz_deserialize -max_total_time=60
```

#### 2. Crash Reproduction

```bash
# Reproduce from crash file
./build-fuzz/fuzz/fuzz_deserialize crash-*
```

#### 3. Coverage Check

```bash
# Coverage build
CFLAGS="-fprofile-instr-generate -fcoverage-mapping" \
CXXFLAGS="-fprofile-instr-generate -fcoverage-mapping" \
cmake -B build-cov -DBUILD_FUZZING=ON

# Generate coverage report after fuzzing
llvm-profdata merge -sparse default.profraw -o default.profdata
llvm-cov show ./build-cov/fuzz/fuzz_deserialize -instr-profile=default.profdata
```

#### Success Criteria
- [ ] No crashes during 5 minutes of fuzzing
- [ ] Fix any bugs discovered
- [ ] CI integration complete
- [ ] Corpus maintenance

---

## Technical Details

### Fuzzing Strategy

1. **Structure-Aware Fuzzing**: Fuzzer that understands serialization format
2. **Coverage-Guided**: Utilize libFuzzer's coverage feedback
3. **Sanitizers**: Use ASan + UBSan simultaneously

### Expected Vulnerability Types

- Buffer overflow
- Integer overflow
- Use-after-free
- Stack overflow (recursive containers)
- Denial of Service (excessive memory allocation)

---

## Checklist

- [ ] Write fuzz_deserialize.cpp
- [ ] Write fuzz_container_deserialize.cpp
- [ ] Write fuzz/CMakeLists.txt
- [ ] Write fuzzing.yaml workflow
- [ ] Generate seed corpus
- [ ] Local fuzzing test
- [ ] CI integration test
- [ ] Fix discovered bugs

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
