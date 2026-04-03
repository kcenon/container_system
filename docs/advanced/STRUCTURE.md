---
doc_id: "CNT-ARCH-006"
doc_title: "Structure Documentation - Container System"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "ARCH"
---

# Structure Documentation - Container System

> **Version:** 0.1.0.0
> **Last Updated:** 2025-10-22
> **Language:** **English** | [한국어](STRUCTURE.kr.md)

---

## Table of Contents

- [Directory Structure](#directory-structure)
- [Namespace Organization](#namespace-organization)
- [File Organization](#file-organization)
- [Component Layout](#component-layout)
- [Build System Structure](#build-system-structure)
- [Test Organization](#test-organization)
- [Documentation Structure](#documentation-structure)

---

## Directory Structure

### Complete Directory Tree

```
container_system/
├── 📄 container.h                          # Main include header (redirect to core/container.h)
│
├── 📁 core/                                # Core container functionality
│   ├── container.h                         # Container class definition
│   ├── container.cpp                       # Container implementation
│   ├── value.h                             # Abstract value interface
│   ├── value.cpp                           # Value base implementation
│   ├── value_types.h                       # Value type enumeration and conversion
│   ├── value_types.cpp                     # Value type utilities
│   └── optimized_container.h               # Performance-optimized container variant
│
├── 📁 values/                              # Value type implementations
│   ├── bool_value.h                        # Boolean value type
│   ├── bool_value.cpp                      # Boolean implementation
│   ├── numeric_value.h                     # Numeric value templates (int, float, double)
│   ├── string_value.h                      # String value type
│   ├── string_value.cpp                    # String implementation
│   ├── bytes_value.h                       # Raw byte array value
│   ├── bytes_value.cpp                     # Bytes implementation
│   ├── container_value.h                   # Nested container value
│   └── container_value.cpp                 # Nested container implementation
│
├── 📁 internal/                            # Internal implementation details
│   ├── variant_value.h                     # std::variant-based value storage
│   ├── variant_value.cpp                   # Variant implementation
│   ├── thread_safe_container.h             # Thread-safe wrapper
│   ├── thread_safe_container.cpp           # Thread-safe implementation
│   ├── simd_processor.h                    # SIMD optimization utilities
│   ├── simd_processor.cpp                  # SIMD implementations (NEON, AVX2, SSE)
│   └── memory_pool.h                       # Memory pooling for performance
│
├── 📁 integration/                         # Integration with ecosystem
│   ├── messaging_integration.h             # messaging_system integration
│   └── messaging_integration.cpp           # Messaging integration implementation
│
├── 📁 include/container/                   # Public headers for integration
│   ├── adapters/                           # Adapter pattern implementations
│   │   └── common_result_adapter.h         # Result<T> error handling adapter
│   └── optimizations/                      # Performance optimization headers
│       └── fast_parser.h                   # Fast serialization parser
│
├── 📁 utilities/                           # Utility functions
│   ├── core/                               # Core utilities
│   │   ├── convert_string.h                # String conversion utilities
│   │   └── formatter.h                     # Formatting utilities
│   ├── conversion/                         # Type conversion
│   │   └── convert_string.h                # String conversion stub
│   └── convert_string_stub.cpp             # Stub implementation
│
├── 📁 tests/                               # Unit tests
│   ├── unit_tests.cpp                      # Core unit tests
│   ├── thread_safety_tests.cpp             # Thread safety validation
│   ├── performance_tests.cpp               # Performance benchmarks
│   ├── benchmark_tests.cpp                 # Detailed benchmarks
│   └── test_messaging_integration.cpp      # Integration tests
│
├── 📁 integration_tests/                   # Integration test suite
│   ├── framework/                          # Test framework
│   │   ├── test_config.h                   # Test configuration
│   │   ├── test_helpers.h                  # Helper functions
│   │   ├── system_fixture.h                # Test fixtures
│   │   └── test_environment_validation.cpp # Environment validation
│   ├── scenarios/                          # Test scenarios
│   │   ├── container_lifecycle_test.cpp    # Container lifecycle tests
│   │   └── value_operations_test.cpp       # Value operation tests
│   ├── performance/                        # Performance tests
│   │   └── serialization_performance_test.cpp # Serialization benchmarks
│   └── failures/                           # Error handling tests
│       └── error_handling_test.cpp         # Error scenario tests
│
├── 📁 benchmarks/                          # Performance benchmarks
│   ├── main_bench.cpp                      # Benchmark entry point
│   ├── container_operations_bench.cpp      # Container operation benchmarks
│   ├── serialization_bench.cpp             # Serialization benchmarks
│   └── value_operations_bench.cpp          # Value operation benchmarks
│
├── 📁 samples/                             # Sample applications
│   ├── basic_usage.cpp                     # Basic container usage
│   ├── thread_safe_example.cpp             # Thread safety examples
│   ├── performance_benchmark.cpp           # Performance demonstration
│   └── run_all_samples.cpp                 # Sample runner
│
├── 📁 examples/                            # Example applications
│   ├── basic_container_example.cpp         # Basic container example
│   ├── advanced_container_example.cpp      # Advanced features example
│   ├── messaging_integration_example.cpp   # Messaging integration
│   └── real_world_scenarios.cpp            # Real-world use cases
│
├── 📁 docs/                                # Documentation
│   ├── API_REFERENCE.md                    # API documentation
│   ├── USER_GUIDE.md                       # User guide
│   ├── PERFORMANCE.md                      # Performance guide
│   ├── FAQ.md                              # Frequently asked questions
│   ├── images/                             # Documentation images
│   └── examples/                           # Code examples
│
├── 📁 cmake/                               # CMake modules
│   └── modules/                            # Custom CMake modules
│
├── 📁 .github/                             # GitHub configuration
│   └── workflows/                          # CI/CD workflows
│       ├── ci.yml                          # Continuous integration
│       ├── coverage.yml                    # Code coverage
│       ├── static-analysis.yml             # Static analysis
│       ├── dependency-security-scan.yml    # Security scanning
│       └── build-Doxygen.yaml              # Documentation build
│
├── 📁 vcpkg_installed/                     # vcpkg dependencies (generated)
│
├── 📄 CMakeLists.txt                       # Root CMake configuration
├── 📄 vcpkg.json                           # vcpkg manifest
├── 📄 vcpkg-configuration.json             # vcpkg configuration
│
├── 📄 build.sh                             # Unix build script
├── 📄 build.bat                            # Windows build script (cmd)
├── 📄 build.ps1                            # Windows build script (PowerShell)
├── 📄 dependency.sh                        # Unix dependency script
├── 📄 dependency.bat                       # Windows dependency script (cmd)
├── 📄 dependency.ps1                       # Windows dependency script (PowerShell)
│
├── 📄 README.md                            # Main README
├── 📄 README.kr.md                         # Korean README
├── 📄 CHANGELOG.md                         # Version changelog
├── 📄 CHANGELOG.kr.md                      # Korean changelog
├── 📄 ARCHITECTURE.md                      # Architecture documentation
├── 📄 STRUCTURE.md                         # This file
├── 📄 INTEGRATION.md                       # Integration guide
├── 📄 MIGRATION.md                         # Migration guide
├── 📄 PERFORMANCE.md                       # Performance documentation
├── 📄 PERFORMANCE.kr.md                    # Korean performance docs
├── 📄 IMPLEMENTATION_SUMMARY.md            # Implementation summary
├── 📄 IMPLEMENTATION_SUMMARY.kr.md         # Korean implementation summary
├── 📄 IMPROVEMENTS.md                      # Improvement plans
├── 📄 IMPROVEMENTS.kr.md                   # Korean improvement plans
├── 📄 SANITIZER_TEST_RESULTS.md            # Sanitizer test results
├── 📄 SANITIZER_TEST_RESULTS.kr.md         # Korean sanitizer results
│
├── 📄 Doxyfile                             # Doxygen configuration
├── 📄 mainpage.dox                         # Doxygen main page
├── 📄 .clang-tidy                          # Clang-tidy configuration
├── 📄 .cppcheck                            # Cppcheck configuration
├── 📄 .gitignore                           # Git ignore rules
└── 📄 LICENSE                              # BSD 3-Clause License
```

---

## Namespace Organization

### Primary Namespace: `container_module`

The container system uses the `container_module` namespace for all public APIs.

```cpp
namespace container_module {
    // Core types
    class value_container;
    class value;
    enum class value_types;

    // Value implementations
    class bool_value;
    template<typename T> class numeric_value;
    class string_value;
    class bytes_value;
    class container_value;

    // Internal implementations
    namespace internal {
        class variant_value;
        class thread_safe_container;
        class memory_pool;
    }

    // SIMD optimizations
    namespace simd {
        class processor;
        // Platform-specific implementations
    }

    // Integration features
    namespace integration {
        class messaging_container_builder;
        // Messaging system integration
    }

    // Adapters (when KCENON_HAS_COMMON_SYSTEM is enabled)
    namespace adapters {
        class serialization_result_adapter;
        class deserialization_result_adapter;
        class container_result_adapter;
    }
}
```

### Namespace Hierarchy

```
container_module::
├── core (implicit - main namespace)
│   ├── value_container
│   ├── value
│   └── value_types
│
├── values (implicit - value implementations in main namespace)
│   ├── bool_value
│   ├── numeric_value<T>
│   ├── string_value
│   ├── bytes_value
│   └── container_value
│
├── internal::
│   ├── variant_value
│   ├── thread_safe_container
│   ├── memory_pool
│   └── fast_parser
│
├── simd::
│   ├── processor
│   ├── neon (ARM)
│   ├── avx2 (x86)
│   └── sse42 (x86)
│
├── integration::
│   ├── messaging_container_builder
│   └── messaging_integration
│
└── adapters:: (optional, with common_system)
    ├── serialization_result_adapter
    ├── deserialization_result_adapter
    └── container_result_adapter
```

---

## File Organization

### Core Module (`core/`)

**Purpose:** Fundamental container and value abstractions

| File | Lines | Purpose |
|------|-------|---------|
| `container.h` | ~300 | Container class definition |
| `container.cpp` | ~800 | Container implementation |
| `value.h` | ~150 | Abstract value interface |
| `value.cpp` | ~200 | Value base implementation |
| `value_types.h` | ~122 | Value type enumeration |
| `value_types.cpp` | ~50 | Type conversion utilities |
| `optimized_container.h` | ~200 | Performance-optimized variant |

**Key Responsibilities:**
- Container lifecycle management
- Header management (source, target, message type)
- Value storage and retrieval
- Serialization coordination

### Values Module (`values/`)

**Purpose:** Concrete value type implementations

| File | Lines | Purpose |
|------|-------|---------|
| `bool_value.h` | ~80 | Boolean value type |
| `bool_value.cpp` | ~60 | Boolean implementation |
| `numeric_value.h` | ~200 | Numeric type templates |
| `string_value.h` | ~100 | String value type |
| `string_value.cpp` | ~150 | String implementation |
| `bytes_value.h` | ~120 | Byte array value |
| `bytes_value.cpp` | ~180 | Bytes implementation |
| `container_value.h` | ~100 | Nested container |
| `container_value.cpp` | ~150 | Nested container impl |

**Supported Types:**
- Boolean: `bool_value`
- Integers: `short_value`, `ushort_value`, `int_value`, `uint_value`, `long_value`, `ulong_value`, `llong_value`, `ullong_value`
- Floating-point: `float_value`, `double_value`
- String: `string_value` (UTF-8)
- Binary: `bytes_value` (raw byte array)
- Nested: `container_value` (hierarchical containers)

### Internal Module (`internal/`)

**Purpose:** Internal implementation details and optimizations

| File | Lines | Purpose |
|------|-------|---------|
| `variant_value.h` | ~250 | std::variant-based storage |
| `variant_value.cpp` | ~300 | Variant implementation |
| `thread_safe_container.h` | ~150 | Thread-safe wrapper |
| `thread_safe_container.cpp` | ~200 | Thread-safe impl |
| `simd_processor.h` | ~300 | SIMD optimizations |
| `simd_processor.cpp` | ~600 | SIMD implementations |
| `memory_pool.h` | ~200 | Memory pooling |

**Key Features:**
- Type-safe variant storage (`std::variant`)
- Thread safety with `std::shared_mutex`
- SIMD acceleration (NEON, AVX2, SSE4.2)
- Memory pooling for performance

### Integration Module (`integration/`)

**Purpose:** Integration with ecosystem components

| File | Lines | Purpose |
|------|-------|---------|
| `messaging_integration.h` | ~200 | messaging_system integration |
| `messaging_integration.cpp` | ~400 | Messaging implementation |

**Integration Points:**
- messaging_system: Legacy container compatibility
- Builder pattern for container construction
- Performance optimizations for messaging

### Public Headers (`include/container/`)

**Purpose:** Public API headers for external integration

| Path | Purpose |
|------|---------|
| `adapters/common_result_adapter.h` | Result<T> error handling adapter |
| `optimizations/fast_parser.h` | Fast serialization parser |

**Adapters Provide:**
- Type-safe error handling with `Result<T>`
- Integration with common_system error codes
- Monadic error composition

---

## Component Layout

### Component Dependency Graph

```
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                     │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│              integration/ (Optional)                    │
│  • messaging_integration.h/cpp                          │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│              include/container/adapters/                │
│  • common_result_adapter.h (Optional)                   │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│                    core/                                │
│  • container.h/cpp                                      │
│  • value.h/cpp                                          │
│  • value_types.h/cpp                                    │
└───┬──────────────────────────────────┬──────────────────┘
    │                                  │
    ▼                                  ▼
┌─────────────────────┐    ┌─────────────────────┐
│      values/        │    │     internal/       │
│  • bool_value       │    │  • variant_value    │
│  • numeric_value    │    │  • thread_safe      │
│  • string_value     │    │  • simd_processor   │
│  • bytes_value      │    │  • memory_pool      │
│  • container_value  │    └─────────────────────┘
└─────────────────────┘
```

### Include Hierarchy

**Public API Headers:**
```cpp
// Main header (redirect)
#include <container/container.h>

// Core headers
#include <container/core/container.h>
#include <container/core/value.h>
#include <container/core/value_types.h>

// Value headers
#include <container/values/bool_value.h>
#include <container/values/numeric_value.h>
#include <container/values/string_value.h>
#include <container/values/bytes_value.h>
#include <container/values/container_value.h>

// Integration headers (optional)
#include <container/integration/messaging_integration.h>

// Adapter headers (optional, with common_system)
#include <container/adapters/common_result_adapter.h>
```

**Internal Headers (for library implementation only):**
```cpp
#include <container/internal/variant_value.h>
#include <container/internal/thread_safe_container.h>
#include <container/internal/simd_processor.h>
#include <container/internal/memory_pool.h>
```

---

## Build System Structure

### CMake Configuration Files

```
container_system/
├── CMakeLists.txt                    # Root CMake configuration
├── cmake/
│   └── modules/
│       ├── FindSIMD.cmake            # SIMD detection
│       └── CompilerOptions.cmake     # Compiler settings
```

### CMake Targets

**Library Targets:**
```cmake
# Main library target
container_system                       # Static library

# Aliases for compatibility
ContainerSystem::container             # Standalone usage
MessagingSystem::container             # messaging_system compatibility
```

**Build Options:**
```cmake
# Feature flags
BUILD_CONTAINERSYSTEM_AS_SUBMODULE    # Build as submodule (OFF)
BUILD_WITH_COMMON_SYSTEM              # Enable common_system integration (ON, defines KCENON_HAS_COMMON_SYSTEM)
ENABLE_MESSAGING_FEATURES             # Messaging optimizations (ON)
ENABLE_EXTERNAL_INTEGRATION           # External integration (ON)
ENABLE_PERFORMANCE_METRICS            # Performance metrics (OFF)

# Testing flags
BUILD_TESTS                           # Build unit tests (ON)
BUILD_CONTAINER_SAMPLES               # Build samples (ON)
CONTAINER_BUILD_BENCHMARKS            # Build benchmarks (OFF)
CONTAINER_BUILD_INTEGRATION_TESTS     # Build integration tests (ON)

# Configuration flags
USE_THREAD_SAFE_OPERATIONS            # Thread-safe operations (ON)
USE_LOCKFREE_BY_DEFAULT               # Lock-free by default (OFF)
BUILD_SHARED_LIBS                     # Build shared libraries (OFF)
BUILD_DOCUMENTATION                   # Build Doxygen docs (ON)
```

### Build Scripts

**Platform-Specific Scripts:**

| Platform | Dependency Script | Build Script |
|----------|-------------------|--------------|
| Unix (Linux/macOS) | `dependency.sh` | `build.sh` |
| Windows (cmd) | `dependency.bat` | `build.bat` |
| Windows (PowerShell) | `dependency.ps1` | `build.ps1` |

**Script Features:**
- Automatic vcpkg installation
- Dependency resolution
- Multi-configuration builds
- Platform detection
- Compiler detection

---

## Test Organization

### Test Structure

```
tests/
├── unit_tests.cpp                      # Core unit tests (Google Test)
├── thread_safety_tests.cpp             # Thread safety validation
├── performance_tests.cpp               # Performance benchmarks
├── benchmark_tests.cpp                 # Detailed benchmarks
└── test_messaging_integration.cpp      # Integration tests

integration_tests/
├── framework/
│   ├── test_config.h                   # Test configuration
│   ├── test_helpers.h                  # Helper utilities
│   ├── system_fixture.h                # Test fixtures
│   └── test_environment_validation.cpp # Environment checks
├── scenarios/
│   ├── container_lifecycle_test.cpp    # Lifecycle scenarios
│   └── value_operations_test.cpp       # Value operation tests
├── performance/
│   └── serialization_performance_test.cpp # Serialization benchmarks
└── failures/
    └── error_handling_test.cpp         # Error handling tests

benchmarks/
├── main_bench.cpp                      # Benchmark entry point
├── container_operations_bench.cpp      # Container benchmarks
├── serialization_bench.cpp             # Serialization benchmarks
└── value_operations_bench.cpp          # Value operation benchmarks
```

### Test Categories

**1. Unit Tests (`tests/unit_tests.cpp`):**
- Container creation and destruction
- Value addition and retrieval
- Serialization and deserialization
- Type safety validation

**2. Thread Safety Tests (`tests/thread_safety_tests.cpp`):**
- Concurrent read operations
- Concurrent write operations
- Read-write concurrency
- ThreadSanitizer validation

**3. Performance Tests (`tests/performance_tests.cpp`):**
- Container creation throughput
- Value operation latency
- Serialization performance
- Memory efficiency

**4. Integration Tests (`integration_tests/`):**
- End-to-end scenarios
- Cross-component integration
- Error handling flows
- Performance under load

**5. Benchmarks (`benchmarks/`):**
- Detailed performance metrics
- Comparative analysis
- SIMD optimization validation
- Memory profiling

---

## Documentation Structure

### Documentation Files

```
docs/
├── API_REFERENCE.md                    # Complete API documentation
├── USER_GUIDE.md                       # User guide and tutorials
├── PERFORMANCE.md                      # Performance guide and benchmarks
├── FAQ.md                              # Frequently asked questions
├── CONTRIBUTING.md                     # Contribution guidelines
├── images/                             # Documentation images
│   ├── architecture.png
│   ├── performance_graph.png
│   └── integration_diagram.png
└── examples/                           # Code examples
    ├── basic_usage.cpp
    ├── advanced_features.cpp
    └── integration_examples.cpp

# Root documentation
README.md                               # Main README (English)
README.kr.md                            # Korean README
CHANGELOG.md                            # Version changelog (English)
CHANGELOG.kr.md                         # Korean changelog
ARCHITECTURE.md                         # Architecture documentation
STRUCTURE.md                            # This file
INTEGRATION.md                          # Integration guide
MIGRATION.md                            # Migration guide from messaging_system
PERFORMANCE.md                          # Performance documentation (English)
PERFORMANCE.kr.md                       # Korean performance docs
IMPLEMENTATION_SUMMARY.md               # Implementation summary (English)
IMPLEMENTATION_SUMMARY.kr.md            # Korean implementation summary
IMPROVEMENTS.md                         # Improvement plans (English)
IMPROVEMENTS.kr.md                      # Korean improvement plans
SANITIZER_TEST_RESULTS.md               # Sanitizer results (English)
SANITIZER_TEST_RESULTS.kr.md            # Korean sanitizer results
```

### Documentation Categories

**1. User Documentation:**
- README.md: Project overview and quick start
- docs/USER_GUIDE.md: Comprehensive user guide
- docs/API_REFERENCE.md: Complete API documentation
- docs/FAQ.md: Common questions and answers

**2. Architecture Documentation:**
- ARCHITECTURE.md: System design and principles
- STRUCTURE.md: Project organization (this file)
- INTEGRATION.md: Integration with ecosystem

**3. Development Documentation:**
- IMPLEMENTATION_SUMMARY.md: Implementation details
- PERFORMANCE.md: Performance characteristics
- performance/BASELINE.md: Performance baselines
- SANITIZER_TEST_RESULTS.md: Testing results

**4. Migration Documentation:**
- MIGRATION.md: Migration from messaging_system
- CHANGELOG.md: Version history
- IMPROVEMENTS.md: Improvement roadmap

**5. Generated Documentation:**
- Doxyfile: Doxygen configuration
- mainpage.dox: Doxygen main page
- build/documents/html/: Generated API docs

---

## Summary

### Project Statistics

| Category | Count | Notes |
|----------|-------|-------|
| **Source Files** | 58 | Core, values, internal, integration |
| **Header Files** | 28 | Public API headers |
| **Implementation Files** | 30 | .cpp files |
| **Test Files** | 11 | Unit, integration, benchmarks |
| **Example Files** | 8 | Samples and examples |
| **Documentation Files** | 20+ | English and Korean versions |
| **Build Scripts** | 6 | Unix and Windows variants |
| **CMake Files** | 1 | Root CMakeLists.txt |

### Lines of Code (Approximate)

| Component | LoC | Percentage |
|-----------|-----|------------|
| Core module | 1,500 | 20% |
| Values module | 1,200 | 16% |
| Internal module | 2,000 | 27% |
| Integration | 600 | 8% |
| Tests | 1,500 | 20% |
| Documentation | 700 | 9% |
| **Total** | **~7,500** | **100%** |

### Key Directories Summary

**Core Functionality:**
- `core/`: Container and value abstractions (1,500 LoC)
- `values/`: Concrete value types (1,200 LoC)
- `internal/`: Optimizations and internals (2,000 LoC)

**Integration:**
- `integration/`: Ecosystem integration (600 LoC)
- `include/container/`: Public headers for external use

**Testing:**
- `tests/`: Unit and integration tests (800 LoC)
- `integration_tests/`: Comprehensive test scenarios (400 LoC)
- `benchmarks/`: Performance benchmarks (300 LoC)

**Examples:**
- `samples/`: Basic usage samples (400 LoC)
- `examples/`: Advanced examples (600 LoC)

**Documentation:**
- Root-level .md files: Project documentation
- `docs/`: Detailed guides and references

---

**Maintainer:** kcenon@naver.com
**License:** BSD 3-Clause
**Version:** 0.1.0.0
**Last Updated:** 2025-10-22
