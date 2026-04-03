---
doc_id: "CNT-PROJ-004"
doc_title: "Container System Project Structure"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "PROJ"
---

# Container System Project Structure

**Last Updated**: 2025-12-10

## Overview

This document provides comprehensive information about the Container System project structure, including directory organization, file descriptions, module dependencies, and build artifacts.

## Directory Tree

```
container_system/
├── 📁 core/                        # Core container functionality
│   ├── concepts.h                  # C++20 concepts for type validation (NEW)
│   ├── container.h                 # Main container class
│   ├── container.cpp               # Container implementation
│   ├── simd_batch.h                # SIMD-friendly batch container (renamed from typed_container, uses TriviallyCopyable concept)
│   ├── typed_container.h           # Deprecated: redirects to simd_batch.h
│   ├── value_types.h               # Value type enumerations
│   ├── value_types.cpp             # Value type implementation
│   ├── value_store.h               # Value storage abstraction
│   └── value_store.cpp             # Value storage implementation
├── 📁 internal/                    # Internal implementations
│   ├── value.h                     # Value class (uses ValueVariantType, ValueVisitor concepts)
│   ├── value.cpp                   # Value implementation
│   ├── thread_safe_container.h     # Thread-safe container (uses KeyValueCallback, MutableKeyValueCallback concepts)
│   ├── thread_safe_container.cpp   # Thread-safe implementation
│   ├── variant_value_factory.h     # Variant value factory (uses Arithmetic concept)
│   ├── memory_pool.h               # Memory pool for allocations
│   ├── simd_processor.h            # SIMD optimization utilities
│   └── simd_processor.cpp          # SIMD implementation
├── 📁 integration/                 # Integration helpers
│   ├── messaging_integration.h     # Messaging integration (uses IntegralType, FloatingPointType, StringLike concepts)
│   └── messaging_integration.cpp   # Messaging implementation
├── 📁 include/container/           # Public API headers (compatibility)
│   ├── 📁 core/                    # Core container functionality
│   │   ├── container.h             # Main container class
│   │   ├── value.h                 # Abstract value base class
│   │   ├── value_types.h           # Value type enumerations
│   │   └── value_factory.h         # Type-safe value creation
│   ├── 📁 values/                  # Concrete value implementations
│   │   ├── primitive_values.h      # bool, char, string
│   │   ├── numeric_values.h        # int8-int64, float, double
│   │   ├── container_value.h       # Nested container support
│   │   └── bytes_value.h           # Raw byte arrays
│   ├── 📁 advanced/                # Advanced features
│   │   ├── variant_value.h         # Polymorphic value storage
│   │   ├── thread_safe_container.h # Thread-safe wrapper
│   │   └── simd_processor.h        # SIMD optimization utilities
│   ├── 📁 serialization/           # Serialization support
│   │   ├── binary_serializer.h     # Binary format
│   │   ├── json_serializer.h       # JSON format
│   │   ├── xml_serializer.h        # XML format
│   │   └── format_detector.h       # Automatic format detection
│   ├── 📁 integration/             # Integration helpers
│   │   ├── messaging_builder.h     # Messaging system builder
│   │   ├── network_serializer.h    # Network optimizations
│   │   └── database_adapter.h      # Database storage adapter
│   └── 📁 adapters/                # Error handling adapters
│       ├── common_result_adapter.h # Result<T> adapters
│       └── error_codes.h           # Error code definitions
├── 📁 src/                         # Implementation files
│   ├── 📁 core/                    # Core implementations
│   │   ├── container.cpp
│   │   ├── value.cpp
│   │   └── value_factory.cpp
│   ├── 📁 values/                  # Value implementations
│   │   ├── primitive_values.cpp
│   │   ├── numeric_values.cpp
│   │   ├── container_value.cpp
│   │   └── bytes_value.cpp
│   ├── 📁 advanced/                # Advanced implementations
│   │   ├── variant_value.cpp
│   │   ├── thread_safe_container.cpp
│   │   └── simd_processor.cpp
│   ├── 📁 serialization/           # Serialization implementations
│   │   ├── binary_serializer.cpp
│   │   ├── json_serializer.cpp
│   │   ├── xml_serializer.cpp
│   │   └── format_detector.cpp
│   ├── 📁 integration/             # Integration implementations
│   │   ├── messaging_builder.cpp
│   │   ├── network_serializer.cpp
│   │   └── database_adapter.cpp
│   └── 📁 adapters/                # Adapter implementations
│       └── common_result_adapter.cpp
├── 📁 examples/                    # Example applications
│   ├── 📁 basic_container_example/ # Basic usage
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── 📁 advanced_container_example/ # Advanced features
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── 📁 real_world_scenarios/    # Industry examples
│   │   ├── financial_trading.cpp
│   │   ├── iot_sensors.cpp
│   │   └── web_api.cpp
│   └── 📁 messaging_integration_example/ # Messaging integration
│       ├── CMakeLists.txt
│       └── main.cpp
├── 📁 tests/                       # Test suite
│   ├── 📁 unit/                    # Unit tests
│   │   ├── container_test.cpp
│   │   ├── value_factory_test.cpp
│   │   ├── serialization_test.cpp
│   │   ├── thread_safety_test.cpp
│   │   └── simd_test.cpp
│   ├── 📁 integration/             # Integration tests
│   │   ├── messaging_integration_test.cpp
│   │   ├── network_integration_test.cpp
│   │   └── database_integration_test.cpp
│   └── 📁 performance/             # Performance benchmarks
│       ├── benchmark_main.cpp
│       ├── container_benchmark.cpp
│       ├── serialization_benchmark.cpp
│       └── simd_benchmark.cpp
├── 📁 docs/                        # Documentation
│   ├── README.md                   # Documentation index
│   ├── FEATURES.md                 # Feature documentation
│   ├── BENCHMARKS.md               # Performance benchmarks
│   ├── PROJECT_STRUCTURE.md        # This file
│   ├── PRODUCTION_QUALITY.md       # Quality metrics
│   ├── ARCHITECTURE.md             # Architecture guide
│   ├── API_REFERENCE.md            # API documentation
│   ├── USER_GUIDE.md               # Usage guide
│   ├── PERFORMANCE.md              # SIMD optimization guide
│   ├── MIGRATION.md                # Migration guide
│   ├── INTEGRATION.md              # Integration guide
│   ├── 📁 guides/                  # User guides
│   │   ├── BUILD_GUIDE.md
│   │   ├── QUICK_START.md
│   │   ├── BEST_PRACTICES.md
│   │   ├── TROUBLESHOOTING.md
│   │   └── FAQ.md
│   ├── 📁 advanced/                # Advanced topics
│   │   ├── SIMD_OPTIMIZATION.md
│   │   ├── THREAD_SAFETY.md
│   │   └── MEMORY_MANAGEMENT.md
│   └── 📁 performance/             # Performance data
│       ├── BASELINE.md
│       └── MEMORY_POOL_PERFORMANCE.md
├── 📁 cmake/                       # CMake modules
│   ├── FindUtilities.cmake         # Find utilities module
│   ├── CompilerOptions.cmake       # Compiler configuration
│   └── Dependencies.cmake          # Dependency management
├── 📁 scripts/                     # Build scripts
│   ├── dependency.sh               # Linux/macOS dependency installer
│   ├── dependency.bat              # Windows dependency installer (CMD)
│   ├── dependency.ps1              # Windows dependency installer (PowerShell)
│   ├── build.sh                    # Linux/macOS build script
│   ├── build.bat                   # Windows build script (CMD)
│   └── build.ps1                   # Windows build script (PowerShell)
├── 📁 benchmarks/                  # Benchmark data
│   ├── BASELINE.md                 # Raw baseline data for CI
│   └── results/                    # Benchmark results
├── 📁 .github/                     # GitHub workflows
│   └── workflows/
│       ├── ci.yml                  # Main CI pipeline
│       ├── coverage.yml            # Coverage reporting
│       ├── static-analysis.yml     # Static analysis
│       ├── dependency-security-scan.yml # Security scanning
│       └── build-Doxygen.yaml      # API documentation generation
├── 📄 CMakeLists.txt               # Main build configuration
├── 📄 vcpkg.json                   # Dependency manifest
├── 📄 .gitignore                   # Git ignore rules
├── 📄 LICENSE                      # BSD 3-Clause license
├── 📄 README.md                    # Main README
└── 📄 README.kr.md                 # Korean README
```

## Core Module Files

### Container Core (`core/`)

#### `concepts.h`
**Purpose**: C++20 concepts for container_system type validation

**Key Features**:
- Compile-time type constraints with clear error messages
- Replaces SFINAE-based constraints
- Integration with common_system concepts

**Requirements**:
- C++20 compiler with concepts support
- GCC 10+, Clang 10+, MSVC 2022+

**Defined Concepts**:

| Category | Concepts |
|----------|----------|
| Type Constraints | `Arithmetic`, `IntegralType`, `FloatingPointType`, `SignedIntegral`, `UnsignedIntegral`, `TriviallyCopyable` |
| Value Types | `ValueVariantType`, `NumericValueType`, `StringLike`, `ByteContainer` |
| Callbacks | `ValueVisitor`, `KeyValueCallback`, `MutableKeyValueCallback`, `ValueMapCallback`, `ConstValueMapCallback` |
| Serialization | `Serializable`, `JsonSerializable` |
| Container | `ContainerValue` |

**Usage Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

// Using TriviallyCopyable for SIMD-friendly containers
template<TriviallyCopyable TValue>
class simd_batch { /* ... */ };  // Renamed from typed_container (Issue #328)

// Using KeyValueCallback for iteration
template<KeyValueCallback Func>
void for_each(Func&& func) const;
```

**Integration Points**:
- `simd_batch.h` - Uses `TriviallyCopyable` concept
- `thread_safe_container.h` - Uses `KeyValueCallback`, `MutableKeyValueCallback`, `ValueMapCallback`, `ConstValueMapCallback`
- `value.h` - Uses `ValueVariantType`, `ValueVisitor`
- `variant_value_factory.h` - Uses `Arithmetic`
- `messaging_integration.h` - Uses `IntegralType`, `FloatingPointType`, `StringLike`

#### `container.h` / `container.cpp`
**Purpose**: Main container class providing header management and value storage

**Key Features**:
- Header management (source, target, message_type)
- Value storage with hash map lookup
- Serialization/deserialization support
- Copy operations (shallow and deep)
- Header swapping for request/response patterns

**Public Interface**:
```cpp
class value_container {
    // Header management
    void set_source(const std::string& id, const std::string& sub_id = "");
    void set_target(const std::string& id, const std::string& sub_id = "");
    void set_message_type(const std::string& type);
    void swap_header();

    // Value management
    void add_value(std::shared_ptr<value> val);
    void set_values(const std::vector<std::shared_ptr<value>>& values);
    std::shared_ptr<value> get_value(const std::string& key) const;
    void reserve_values(size_t count);

    // Serialization
    std::string serialize() const;
    std::vector<uint8_t> serialize_array() const;
    std::string to_json() const;
    std::string to_xml() const;
};
```

#### `value.h` / `value.cpp`
**Purpose**: Abstract base class for all value types

**Key Features**:
- Type enumeration and identification
- Virtual interface for serialization
- String conversion support
- Key-value pairing

**Public Interface**:
```cpp
class value {
    virtual value_types type() const = 0;
    virtual std::string key() const = 0;
    virtual std::string to_string() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::shared_ptr<value> copy() const = 0;
};
```

#### `value_types.h`
**Purpose**: Type enumeration and type identification

**Key Features**:
- 16 distinct value types (null to array)
- Type code mapping ('0' to 'f')
- Type name conversion utilities

**Enumerations**:
```cpp
enum class value_types : char {
    null_value = '0',
    bool_value = '1',
    char_value = '2',
    // ... (16 types total, indices 0-15)
    string_value = 'f'
};
```

#### `value_factory.h` / `value_factory.cpp`
**Purpose**: Factory pattern for type-safe value creation

**Key Features**:
- Type-safe value construction
- Template-based type inference
- Static factory methods

**Public Interface**:
```cpp
class value_factory {
    static std::shared_ptr<value> create(
        const std::string& key,
        value_types type,
        const std::string& data
    );

    static std::shared_ptr<bool_value> create_bool(const std::string& key, bool val);
    static std::shared_ptr<string_value> create_string(const std::string& key, const std::string& val);
    static std::shared_ptr<int64_value> create_int64(const std::string& key, int64_t val);
    static std::shared_ptr<double_value> create_double(const std::string& key, double val);
};
```

## Value Implementation Files

### Primitive Values (`include/container/values/primitive_values.h`)

**Classes**: `null_value`, `bool_value`, `char_value`, `string_value`

**Features**:
- Small String Optimization (SSO) for short strings
- Zero-allocation for null values
- Single-byte storage for bool/char

### Numeric Values (`include/container/values/numeric_values.h`)

**Classes**: `int8_value`, `uint8_value`, `int16_value`, `uint16_value`, `int32_value`, `uint32_value`, `int64_value`, `uint64_value`, `float_value`, `double_value`

**Features**:
- SIMD-optimized operations
- Efficient binary serialization
- Type-safe conversions

### Container Value (`include/container/values/container_value.h`)

**Purpose**: Support for nested containers

**Features**:
- Recursive serialization
- Deep copy support
- Type-safe container nesting

### Bytes Value (`include/container/values/bytes_value.h`)

**Purpose**: Raw byte array storage

**Features**:
- Efficient binary data handling
- Zero-copy move semantics
- Size-prefixed serialization

## Advanced Features

### Variant Value (`include/container/advanced/variant_value.h`)

**Purpose**: Polymorphic value storage using `std::variant`

**Features**:
- Type-safe variant storage
- Compile-time type checking
- Visitor pattern support

### Thread-Safe Container (`include/container/advanced/thread_safe_container.h`)

**Purpose**: Thread-safe wrapper for concurrent access

**Features**:
- Read-write lock for safe concurrent access
- Lock-free read operations
- Synchronized write operations

**Public Interface**:
```cpp
class thread_safe_container {
    std::shared_ptr<value> get_value(const std::string& key) const;
    void set_value(const std::string& key, value_types type, const std::string& data);
    void add_value(std::shared_ptr<value> val);
    std::string serialize() const;
};
```

### SIMD Processor (`include/container/advanced/simd_processor.h`)

**Purpose**: SIMD acceleration for numeric operations

**Features**:
- ARM NEON support (Apple Silicon)
- x86 AVX2 support (Intel/AMD)
- Automatic platform detection
- Fallback to scalar operations

**Public Interface**:
```cpp
class simd_processor {
    std::vector<double> process_array(const std::vector<double>& input);
    std::vector<float> process_array(const std::vector<float>& input);
    std::vector<int64_t> process_array(const std::vector<int64_t>& input);
    bool is_simd_available() const;
    std::string get_simd_type() const;
};
```

## Serialization Modules

### Binary Serializer (`include/container/serialization/binary_serializer.h`)

**Purpose**: High-performance binary serialization

**Features**:
- Minimal overhead (~10%)
- Fast serialization (1.8M ops/s)
- Efficient deserialization (2.1M ops/s)

### JSON Serializer (`include/container/serialization/json_serializer.h`)

**Purpose**: Human-readable JSON format

**Features**:
- Pretty-printing support
- Type-safe JSON generation
- nlohmann::json integration

### XML Serializer (`include/container/serialization/xml_serializer.h`)

**Purpose**: Schema-validated XML format

**Features**:
- Namespace support
- Schema validation
- pugixml integration

### Format Detector (`include/container/serialization/format_detector.h`)

**Purpose**: Automatic format detection for deserialization

**Features**:
- Binary/JSON/XML detection
- Magic number validation
- Fallback to binary format

## Integration Modules

### Messaging Builder (`include/container/integration/messaging_builder.h`)

**Purpose**: Fluent API for messaging system integration

**Features**:
- Method chaining builder pattern
- Type-safe construction
- Optimization hints (speed, network)
- Nested container support

**Public Interface**:
```cpp
class messaging_container_builder {
    messaging_container_builder& source(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& target(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& message_type(const std::string& type);
    messaging_container_builder& add_value(const std::string& key, const auto& value);
    messaging_container_builder& optimize_for_speed();
    messaging_container_builder& optimize_for_network();
    std::shared_ptr<value_container> build();
};
```

### Network Serializer (`include/container/integration/network_serializer.h`)

**Purpose**: Network-optimized serialization

**Features**:
- Compression for large payloads
- Network byte order handling
- Streaming support

### Database Adapter (`include/container/integration/database_adapter.h`)

**Purpose**: Database storage integration

**Features**:
- BLOB serialization
- Schema mapping
- Query builder integration

## Test Organization

### Unit Tests (`tests/unit/`)

**Coverage**: Core functionality, individual components

**Key Tests**:
- `container_test.cpp`: Container operations, header management
- `value_factory_test.cpp`: Value creation, type safety
- `serialization_test.cpp`: Format serialization/deserialization
- `thread_safety_test.cpp`: Concurrent access patterns
- `simd_test.cpp`: SIMD operations, platform detection

**Test Framework**: Google Test
**Total Test Cases**: 123+

### Integration Tests (`tests/integration/`)

**Coverage**: Cross-module integration, ecosystem compatibility

**Key Tests**:
- `messaging_integration_test.cpp`: Messaging system integration
- `network_integration_test.cpp`: Network system integration
- `database_integration_test.cpp`: Database system integration

### Performance Tests (`tests/performance/`)

**Coverage**: Benchmarks, performance regression detection

**Key Benchmarks**:
- `container_benchmark.cpp`: Container creation, value operations
- `serialization_benchmark.cpp`: Format performance comparison
- `simd_benchmark.cpp`: SIMD vs scalar performance

**Benchmark Framework**: Google Benchmark
**Measurement Tool**: High-resolution timers

## Build Artifacts

### Debug Build

```
build/
├── bin/
│   ├── examples/
│   │   ├── basic_container_example
│   │   ├── advanced_container_example
│   │   └── messaging_integration_example
│   └── tests/
│       ├── unit_tests
│       ├── integration_tests
│       └── performance_tests
└── lib/
    ├── libcontainer.a              # Static library
    └── libcontainer.so             # Shared library (Linux)
```

### Release Build

```
build/
├── bin/
│   └── [same as debug]
├── lib/
│   └── [optimized libraries]
└── docs/
    └── html/                       # Doxygen documentation
        └── index.html
```

## Module Dependencies

### Internal Dependencies

```
utilities_module (external dependency)
    │
    └──> core (container, value, value_types, value_factory)
            │
            ├──> values (primitive, numeric, container, bytes)
            │      │
            │      └──> advanced (variant, thread_safe, simd)
            │
            ├──> serialization (binary, json, xml, format_detector)
            │
            ├──> integration (messaging_builder, network, database)
            │
            └──> adapters (common_result_adapter, error_codes)
```

### External Dependencies

| Dependency | Purpose | Version | Required |
|-----------|---------|---------|----------|
| **utilities_module** | String conversion, system utilities | Latest | Yes |
| **nlohmann_json** | JSON serialization | 3.11+ | Yes |
| **pugixml** | XML serialization | 1.13+ | Yes |
| **googletest** | Unit testing framework | 1.14+ | Test only |
| **google-benchmark** | Performance benchmarking | 1.8+ | Test only |

> **Note**: This project requires C++20 features:
> - **C++20 Concepts**: Type validation (requires GCC 10+, Clang 10+, or MSVC 2022+)
> - **`std::format`**: String formatting (requires GCC 13+, Clang 14+, or MSVC 19.29+)

### Optional Integration Dependencies

| Dependency | Purpose | Integration Point |
|-----------|---------|------------------|
| **messaging_system** | Message encapsulation | `messaging_builder.h` |
| **network_system** | Network transport | `network_serializer.h` |
| **database_system** | Persistent storage | `database_adapter.h` |

## Build Configuration

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `ENABLE_MESSAGING_FEATURES` | Enhanced messaging optimizations | ON |
| `ENABLE_PERFORMANCE_METRICS` | Real-time performance monitoring | OFF |
| `ENABLE_EXTERNAL_INTEGRATION` | External system callbacks | ON |
| `BUILD_CONTAINER_EXAMPLES` | Build example applications | ON |
| `BUILD_CONTAINER_SAMPLES` | Build sample programs | ON |
| `USE_THREAD_SAFE_OPERATIONS` | Enable thread-safe operations | ON |
| `ENABLE_SIMD` | Enable SIMD optimizations | ON |
| `BUILD_TESTING` | Build test suite | ON |

### Platform-Specific Build

#### Linux/macOS
```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_SIMD=ON \
  -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
```

#### Windows (PowerShell)
```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_SIMD=ON
cmake --build build --config Release
```

## File Naming Conventions

### Header Files
- **Public API**: `include/container/<module>/<name>.h`
- **Private Headers**: `src/<module>/<name>_internal.h`

### Implementation Files
- **Source Files**: `src/<module>/<name>.cpp`

### Test Files
- **Unit Tests**: `tests/unit/<module>_test.cpp`
- **Integration Tests**: `tests/integration/<module>_integration_test.cpp`
- **Benchmarks**: `tests/performance/<module>_benchmark.cpp`

### Documentation Files
- **Markdown**: `docs/<TOPIC>.md`
- **Korean Translation**: `docs/<TOPIC>.kr.md`

## Code Organization Best Practices

### Header Organization
```cpp
// System includes
#include <string>
#include <vector>
#include <memory>

// Standard library includes (C++20)
#include <format>

// Project includes
#include "container/core/value.h"
#include "container/values/primitive_values.h"

// Local includes
#include "internal_helper.h"
```

### Namespace Structure
```cpp
namespace container_module {
    // Core classes
    class value_container { };
    class value { };

    namespace integration {
        // Integration helpers
        class messaging_container_builder { };
    }

    namespace adapters {
        // Error handling adapters
        class serialization_result_adapter { };
    }
}
```

### File Size Guidelines
- **Header Files**: <500 lines
- **Implementation Files**: <1000 lines
- **Test Files**: <800 lines per module
- **Example Files**: <300 lines

## See Also

- [FEATURES.md](FEATURES.md) - Complete feature documentation
- [BENCHMARKS.md](BENCHMARKS.md) - Performance benchmarks
- [PRODUCTION_QUALITY.md](PRODUCTION_QUALITY.md) - Quality metrics
- [ARCHITECTURE.md](ARCHITECTURE.md) - Architecture guide
- [BUILD_GUIDE.md](guides/BUILD_GUIDE.md) - Detailed build instructions

---

**Last Updated**: 2025-12-10
**Version**: 0.1.1.0
