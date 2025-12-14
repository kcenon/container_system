[![CI](https://github.com/kcenon/container_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/ci.yml)
[![Code Coverage](https://github.com/kcenon/container_system/actions/workflows/coverage.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/kcenon/container_system/branch/main/graph/badge.svg)](https://codecov.io/gh/kcenon/container_system)
[![Static Analysis](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml)
[![Security Scan](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml)
[![Documentation](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml)

# Container System

> **Language:** **English** | [한국어](README_KO.md)

## Overview

The Container System is a high-performance C++17 type-safe container framework designed for comprehensive data management in messaging systems and general-purpose applications. Built with a modular, interface-based architecture featuring SIMD optimizations and seamless ecosystem integration.

**Key Highlights**:
- **Type Safety**: Strongly-typed value system with compile-time checks
- **High Performance**: SIMD-accelerated operations (1.8M serializations/sec, 25M SIMD ops/sec)
- **Well-Tested**: Perfect RAII score (20/20), zero data races, comprehensive testing
- **Cross-Platform**: Native support for Linux, macOS, Windows with optimized build scripts
- **Multiple Formats**: Binary, JSON, XML serialization with automatic format detection

### Mission

Making high-performance data serialization **accessible**, **type-safe**, and **efficient** for developers worldwide.

## Quick Start

### Basic Usage Example

```cpp
#include <kcenon/container/core/container.h>
#include <kcenon/container/integration/messaging_builder.h>

using namespace container_module;

int main() {
    // Create container using builder pattern
    auto container = messaging_container_builder()
        .source("trading_engine", "session_001")
        .target("risk_monitor", "main")
        .message_type("market_data")
        .add_value("symbol", "AAPL")
        .add_value("price", 175.50)
        .add_value("volume", 1000000)
        .optimize_for_speed()
        .build();

    // High-performance binary serialization
    std::string binary_data = container->serialize();  // 1.8M ops/sec

    // Human-readable JSON
    std::string json_data = container->to_json();

    // Deserialize
    auto restored = std::make_shared<value_container>(binary_data);
    auto price = restored->get_value("price");

    return 0;
}
```

### Requirements

| Dependency | Version | Required | Description |
|------------|---------|----------|-------------|
| C++20 Compiler | GCC 10+ / Clang 10+ / Apple Clang 12+ / MSVC 2022+ | Yes | C++20 Concepts support required |
| CMake | 3.20+ | Yes | Build system |
| common_system | latest | Yes | C++20 Concepts and common interfaces |
| vcpkg | latest | Optional | Package management (recommended) |

### Installation

#### Step 1: Clone Dependencies

```bash
# Clone common_system (required)
git clone https://github.com/kcenon/common_system.git

# Clone container_system
git clone https://github.com/kcenon/container_system.git
```

#### Step 2: Install Dependencies and Build

```bash
cd container_system

# Install dependencies (cross-platform)
./scripts/dependency.sh      # Linux/macOS
# or
scripts\dependency.bat       # Windows (CMD)
.\scripts\dependency.ps1     # Windows (PowerShell)

# Build project
./scripts/build.sh           # Linux/macOS
# or
scripts\build.bat            # Windows (CMD)
.\scripts\build.ps1          # Windows (PowerShell)

# Run examples
./build/examples/basic_container_example
```

## Core Features

### Type-Safe Value System
- **15 built-in types**: From null to nested containers
- **Compile-time checks**: Template metaprogramming ensures type safety
- **Runtime validation**: Type checking during deserialization
- **Zero overhead**: No runtime cost for type safety

### High-Performance Serialization
- **Binary format**: 1.8M ops/sec with ~10% overhead
- **JSON format**: 950K ops/sec, human-readable
- **XML format**: 720K ops/sec with schema validation
- **Auto-detection**: Automatic format identification

### SIMD Acceleration
- **ARM NEON**: 3.2x speedup on Apple Silicon (M1/M2/M3)
- **x86 AVX2**: 2.5x speedup on Intel/AMD processors
- **Auto-detection**: Platform-specific optimization selection
- **25M ops/sec**: Numeric array processing throughput

### Thread Safety
- **Lock-free reads**: Concurrent read operations without synchronization
- **Thread-safe wrapper**: Optional `thread_safe_container` for writes
- **Linear scaling**: 7.5x throughput with 8 threads
- **Zero data races**: ThreadSanitizer validated

### Production Quality
- **Perfect RAII**: 20/20 score (Grade A+), highest in ecosystem
- **Zero memory leaks**: AddressSanitizer clean
- **Zero data races**: ThreadSanitizer validated
- **123+ tests**: Comprehensive test coverage
- **85%+ coverage**: Line coverage with continuous monitoring

📚 **[Complete Features →](docs/FEATURES.md)**

## Performance Highlights

*Benchmarks on Apple M1 (8 cores, ARM NEON), macOS 26.1, Release build*

| Operation | Throughput | Notes |
|-----------|-----------|-------|
| **Container Creation** | 2M/sec | Empty container with header |
| **Value Addition** | 4.5M/sec | Numeric values with SIMD |
| **Binary Serialization** | 1.8M/sec | 1KB container, ~10% overhead |
| **JSON Serialization** | 950K/sec | Pretty-print enabled |
| **SIMD Operations** | 25M/sec | ARM NEON acceleration |
| **Move Operations** | 4.2M/sec | Zero-copy semantics |

**Platform Comparison**:

| Platform | Architecture | Binary Ser. | SIMD Speedup |
|----------|-------------|-------------|--------------|
| Apple M1 | ARM64 | 1.8M/sec | 3.2x (NEON) |
| Intel i7-12700K | x64 | 1.6M/sec | 2.5x (AVX2) |
| AMD Ryzen 9 | x64 | 1.55M/sec | 2.3x (AVX2) |

⚡ **[Full Benchmarks →](docs/BENCHMARKS.md)**

## Architecture Overview

```
                    ┌─────────────────┐
                    │ utilities_module│
                    │   (Foundation)  │
                    └────────┬────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
┌────────────────┐  ┌────────────────┐  ┌────────────────┐
│ container_system│  │messaging_system│  │ network_system │
│  (This Project) │◄─│   (Consumer)   │◄─│  (Transport)   │
└────────────────┘  └────────────────┘  └────────────────┘
         │
         └──────────────────────┐
                                ▼
                    ┌────────────────────┐
                    │  database_system   │
                    │     (Storage)      │
                    └────────────────────┘
```

**Integration Benefits**:
- **Type-safe messaging**: Prevents runtime errors
- **Performance-optimized**: SIMD acceleration for high-throughput
- **Universal serialization**: Binary, JSON, XML for diverse needs
- **Unified data model**: Consistent structure across ecosystem

🏗️ **[Architecture Guide →](docs/ARCHITECTURE.md)**

## Documentation

### Getting Started
- 📖 [Quick Start Guide](docs/guides/QUICK_START.md)
- 🔧 [Build Guide](docs/guides/BUILD_GUIDE.md)
- ✅ [Best Practices](docs/guides/BEST_PRACTICES.md)
- 🔍 [Troubleshooting](docs/guides/TROUBLESHOOTING.md)

### Core Documentation
- 📚 [Features](docs/FEATURES.md) - Complete feature documentation
- ⚡ [Benchmarks](docs/BENCHMARKS.md) - Performance analysis
- 📦 [Project Structure](docs/PROJECT_STRUCTURE.md) - Code organization
- 🏆 [Production Quality](docs/PRODUCTION_QUALITY.md) - Quality metrics

### Technical Guides
- 🏛️ [Architecture](docs/ARCHITECTURE.md) - System design and patterns
- 📘 [API Reference](docs/API_REFERENCE.md) - Complete API documentation
- 📗 [User Guide](docs/USER_GUIDE.md) - Usage patterns and value types
- 🚀 [Performance](docs/PERFORMANCE.md) - SIMD optimization guide

### Troubleshooting
- 🔁 [Troubleshooting Guide](docs/guides/TROUBLESHOOTING.md) consolidates the most common `Result<T>` failures (serialization, SIMD detection, integrations) and provides a quick checklist for interpreting `err.code` / `err.message`.
- 🔗 [Integration](docs/INTEGRATION.md) - Ecosystem integration

### Development
- 🤝 [Contributing](docs/CONTRIBUTING.md) - Contribution guidelines
- 🔄 [Migration](docs/MIGRATION.md) - Migration from messaging_system
- 📋 [FAQ](docs/guides/FAQ.md) - Frequently asked questions

**Language Support**: Most documents available in English and Korean (`*_KO.md`)

## Value Types

The container system supports 15 distinct value types:

| Category | Types | Size |
|----------|-------|------|
| **Primitive** | null, bool, char | 0-1 byte |
| **Integers** | int8, uint8, int16, uint16, int32, uint32, int64, uint64 | 1-8 bytes |
| **Floating** | float, double | 4-8 bytes |
| **Complex** | bytes, container, string | Variable |

**Example**:
```cpp
using namespace container_module;

// Create values using factory
auto int_val = value_factory::create_int64("id", 12345);
auto str_val = value_factory::create_string("name", "John Doe");
auto dbl_val = value_factory::create_double("balance", 1500.75);
auto bool_val = value_factory::create_bool("active", true);

// Add to container
container->add_value(int_val);
container->add_value(str_val);
container->add_value(dbl_val);
container->add_value(bool_val);
```

📚 **[Value Types Details →](docs/FEATURES.md#value-types)**

## Ecosystem Integration

### With Messaging System
```cpp
#include <messaging_system/messaging_client.h>

auto client = std::make_shared<messaging_client>("client_01");

auto message = messaging_container_builder()
    .source("client_01", "session_123")
    .target("server", "main")
    .message_type("request")
    .add_value("action", "query")
    .build();

client->send_container(message);
```

### With Network System
```cpp
#include <network_system/tcp_client.h>

auto tcp_client = network_system::TcpClient::create("localhost", 8080);

// Serialize and send
std::string data = container->serialize();
tcp_client->send(data);

// Receive and deserialize
auto received = tcp_client->receive();
auto restored = std::make_shared<value_container>(received);
```

### With Database System
```cpp
#include <database_system/database_manager.h>

database_manager db;
db.connect("host=localhost dbname=app");

// Store as BLOB
std::string data = container->serialize();
db.insert_query("INSERT INTO messages (data) VALUES (?)", data);
```

🌐 **[Integration Guide →](docs/INTEGRATION.md)**

## Building

### Basic Build

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run tests
cd build && ctest
```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `ENABLE_MESSAGING_FEATURES` | Messaging optimizations | ON |
| `ENABLE_PERFORMANCE_METRICS` | Performance monitoring | OFF |
| `ENABLE_SIMD` | SIMD optimizations | ON |
| `BUILD_CONTAINER_EXAMPLES` | Build examples | ON |
| `BUILD_TESTING` | Build tests | ON |

### CMake Integration

```cmake
# Add as subdirectory
add_subdirectory(container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)

# Or using FetchContent
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(container_system)
```

🔧 **[Build Guide →](docs/guides/BUILD_GUIDE.md)**

## Platform Support

| Platform | Architecture | Compiler | SIMD | Status |
|----------|-------------|----------|------|--------|
| **Linux** | x86_64, ARM64 | GCC 9+, Clang 10+ | AVX2, NEON | ✅ |
| **macOS** | x86_64, ARM64 (Apple Silicon) | Apple Clang, Clang | AVX2, NEON | ✅ |
| **Windows** | x86, x64 | MSVC 2019+, Clang | AVX2 | ✅ |

**Native Build Scripts**:
- Linux/macOS: `scripts/dependency.sh`, `scripts/build.sh`
- Windows: `scripts/dependency.bat`, `scripts/build.bat`, `scripts/dependency.ps1`, `scripts/build.ps1`

## Thread Safety

### Concurrent Read Operations

**Guarantee**: Thread-safe without synchronization

```cpp
// Multiple threads can safely read
std::vector<std::thread> readers;
for (int i = 0; i < 8; ++i) {
    readers.emplace_back([&container]() {
        auto value = container->get_value("price");
        // Safe concurrent read
    });
}
```

**Performance**: 7.5x speedup with 8 threads (linear scaling)

### Concurrent Write Operations

**Use**: `thread_safe_container` for synchronized writes

```cpp
#include <kcenon/container/advanced/thread_safe_container.h>

auto safe_container = std::make_shared<thread_safe_container>(container);

std::vector<std::thread> workers;
for (int i = 0; i < 4; ++i) {
    workers.emplace_back([&safe_container, i]() {
        safe_container->set_value("thread_" + std::to_string(i),
                                 int32_value,
                                 std::to_string(i));
    });
}
```

**Validation**: Zero data races (ThreadSanitizer verified)

## Error Handling

### Hybrid Adapter Pattern

- **Internal operations**: C++ exceptions for performance
- **External API**: Result&lt;T&gt; for type safety

```cpp
#include <kcenon/container/adapters/common_result_adapter.h>
using namespace container::adapters;

// Serialization with Result<T>
auto result = serialization_result_adapter::serialize(*container);
if (!result) {
    std::cerr << "Error: " << result.error().message << "\n";
    return -1;
}
auto data = result.value();

// Container operations with Result<T>
auto get_result = container_result_adapter::get_value<double>(container, "price");
if (!get_result) {
    std::cerr << "Error: " << get_result.error().message << "\n";
}
```

**Error Codes**: -400 to -499 (centralized in common_system)

## Contributing

We welcome contributions! Please see our [Contributing Guide](docs/CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

### Code Style

- Follow modern C++ best practices
- Use RAII and smart pointers
- Maintain formatting (clang-format provided)
- Write comprehensive tests

## Support & Contributing

- 💬 [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- 🐛 [Issue Tracker](https://github.com/kcenon/container_system/issues)
- 🤝 [Contributing Guide](docs/CONTRIBUTING.md)
- 📧 Email: kcenon@naver.com

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by modern serialization frameworks and high-performance computing practices
- Built with C++20 features (GCC 11+, Clang 14+, MSVC 2022+) for maximum performance and safety
- Maintained by kcenon@naver.com

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>
