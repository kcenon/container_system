<!-- BSD 3-Clause License
     Copyright (c) 2025, kcenon
     See the LICENSE file in the project root for full license information. -->

# Contributing to container_system

Thank you for your interest in contributing to the container_system library! This document provides guidelines and instructions for contributing code, documentation, and improvements.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Development Setup](#development-setup)
3. [Code Style Guidelines](#code-style-guidelines)
4. [Testing Guidelines](#testing-guidelines)
5. [Documentation Standards](#documentation-standards)
6. [Contribution Areas](#contribution-areas)
7. [Submitting Changes](#submitting-changes)
8. [Code Review Process](#code-review-process)

## Prerequisites

Before you can develop for container_system, ensure you have the following installed:

### Required

- **C++20 Compiler**: Modern compiler with C++20 Concepts support
  - GCC 10+ / Clang 10+ / Apple Clang 12+ / MSVC 2022+

- **CMake**: Version 3.20 or later
  ```bash
  cmake --version  # Should be >= 3.20
  ```

- **[common_system](https://github.com/kcenon/common_system)**: Required dependency
  - Will be fetched automatically via CMake FetchContent

### Optional (for full feature support)

- **GoogleTest**: For running and writing tests
- **Doxygen**: For generating API documentation (Version 1.8.15 or later)
- **clang-tidy**: For static analysis
- **vcpkg**: For dependency management (recommended)

## Development Setup

### Clone and Build

```bash
# Clone the repository
git clone https://github.com/kcenon/container_system.git
cd container_system

# Build (common_system will be fetched automatically)
./scripts/build.sh           # Linux/macOS
# or
scripts\build.bat            # Windows

# Run tests
cd build && ctest --verbose
```

### Alternative: Local Development with common_system

If you're developing both `container_system` and `common_system` simultaneously:

```bash
# Clone both repositories as siblings
git clone https://github.com/kcenon/common_system.git
git clone https://github.com/kcenon/container_system.git

cd container_system

# CMake will detect ../common_system automatically
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Code Style Guidelines

### Naming Conventions

```cpp
// Namespaces: lowercase with underscores
namespace container_module {
    // ...
}

// Classes/Structs: lowercase with underscores
class value_container {
    // ...
};

// Functions: lowercase with underscores
template<typename T>
Result<T> serialize_value(const T& value);

// Variables: lowercase with underscores
int item_count = 0;
std::string container_name = "default";

// Member variables: trailing underscore for private/protected
class MyClass {
private:
    int value_;
    std::string name_;
};
```

### Code Formatting

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 100 characters maximum (soft limit, hard limit at 120)
- **Braces**: Allman style (opening brace on new line)

### Modern C++ Practices

- Use `Result<T>` pattern from common_system for error handling
- Prefer `std::optional<T>` over raw pointers for optional values
- Leverage `constexpr` where possible
- Use RAII principles consistently
- Document thread safety guarantees

## Testing Guidelines

### Test Structure

Tests are organized in the `tests/` directory:

```
tests/
├── unit/
│   ├── container_test.cpp
│   ├── serialization_test.cpp
│   └── value_types_test.cpp
└── integration/
    └── simd_tests.cpp
```

### Writing Tests

```cpp
#include <container/container.h>
#include <gtest/gtest.h>

using namespace container_module;

TEST(ContainerTest, BasicSerialization) {
    auto container = messaging_container_builder()
        .add_value("key", "value")
        .build();

    std::string serialized = container->serialize();
    auto restored = std::make_shared<value_container>(serialized);

    EXPECT_EQ(restored->get_value("key"), "value");
}
```

### Running Tests

```bash
# Build and run all tests
cmake --build build
cd build && ctest --verbose

# Run specific test
ctest -R "ContainerTest" --verbose
```

## Documentation Standards

All public APIs must be documented using Doxygen-compatible comments:

```cpp
/**
 * @brief Short description of the function.
 *
 * Longer description explaining the purpose and behavior.
 *
 * @tparam T The value type
 * @param value The value to serialize
 * @return Result<std::string> containing serialized data on success
 *
 * @note Thread safety: This function is thread-safe for read operations.
 */
template<typename T>
Result<std::string> serialize(const T& value);
```

## Contribution Areas

### 1. Serialization Formats
- Add new serialization formats (e.g., Protocol Buffers, CBOR)
- Improve existing format performance
- Add format auto-detection capabilities

### 2. SIMD Optimizations
- Extend SIMD support for additional architectures
- Optimize batch operations
- Profile and benchmark improvements

### 3. Value Types
- Add new value types
- Improve type conversion utilities
- Enhance type safety mechanisms

### 4. Thread Safety
- Improve concurrent read/write operations
- Add new thread-safe container variants
- Profile and optimize locking mechanisms

### 5. Documentation & Examples
- Improve API documentation
- Add usage examples
- Write tutorials and guides

## Submitting Changes

### Pre-Submission Checklist

1. **Format Code**: Run clang-format
   ```bash
   find include tests -name "*.h" -o -name "*.cpp" | xargs clang-format -i
   ```

2. **Compile and Test**:
   ```bash
   cmake --build build
   cd build && ctest --verbose
   ```

3. **Documentation**: Update relevant documentation files

4. **License Header**: Ensure files have correct license header
   ```cpp
   // BSD 3-Clause License
   // Copyright (c) 2025, kcenon
   // See the LICENSE file in the project root for full license information.
   ```

### Creating a Pull Request

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Commit changes with descriptive messages
4. Push to your fork: `git push origin feature/your-feature-name`
5. Open a pull request on the main repository

### Commit Message Format

Follow conventional commits:

```
type(scope): brief description

Longer explanation of the changes.

Closes #123
```

Examples:
- `feat(serialization): add CBOR format support`
- `fix(container): prevent data race in concurrent access`
- `docs(api): improve value_container documentation`
- `test(simd): add ARM NEON benchmark tests`
- `perf(binary): optimize serialization throughput`

## Code Review Process

### Review Criteria

All submissions are reviewed based on:

1. **Correctness**: Code works as intended, no undefined behavior
2. **Performance**: No unnecessary runtime overhead, especially for serialization
3. **C++20 Compatibility**: Works with C++20 Concepts
4. **Documentation**: Complete Doxygen comments with examples
5. **Testing**: Comprehensive tests with good coverage
6. **Thread Safety**: Clear documentation of thread safety guarantees

### Timeline

- Initial feedback: within 2-5 business days
- Revised submissions: reviewed within 2-3 business days
- Merge after approval: immediate

## Getting Help

- **Issues**: File bugs or feature requests on GitHub
- **Discussions**: Use GitHub Discussions for design questions
- **Email**: Contact kcenon@naver.com

## License

By submitting code to container_system, you agree that your contributions will be licensed under the BSD 3-Clause License. See LICENSE file for details.

---

Thank you for contributing to container_system! Your effort helps make high-performance data serialization accessible to everyone.
