# Quick Start Guide

Get started with container_system in 5 minutes.

## Prerequisites

| Dependency | Version | Required | Description |
|------------|---------|----------|-------------|
| C++20 Compiler | GCC 10+ / Clang 10+ / Apple Clang 12+ / MSVC 2022+ | Yes | C++20 Concepts support required |
| CMake | 3.20+ | Yes | Build system |
| common_system | latest | Yes | C++20 Concepts and common interfaces |
| vcpkg | latest | Optional | Package management (recommended) |

## Quick Install

### Step 1: Clone Dependencies

```bash
# Clone common_system (required dependency)
git clone https://github.com/kcenon/common_system.git

# Clone container_system
git clone https://github.com/kcenon/container_system.git
```

### Step 2: Install Dependencies and Build

**Linux/macOS:**
```bash
cd container_system

# Install dependencies
./scripts/dependency.sh

# Build
./scripts/build.sh
```

**Windows (CMD):**
```batch
cd container_system

rem Install dependencies
scripts\dependency.bat

rem Build
scripts\build.bat
```

**Windows (PowerShell):**
```powershell
cd container_system

# Install dependencies
.\scripts\dependency.ps1

# Build
.\scripts\build.ps1
```

### Step 3: Verify Installation

```bash
# Run examples
./build/examples/basic_container_example
```

## Basic Usage

### Creating and Using a Container

```cpp
#include <container/core/container.h>
#include <container/integration/messaging_builder.h>

using namespace container_module;

int main() {
    // Create container using builder pattern
    auto container = messaging_container_builder()
        .source("my_app", "session_001")
        .target("service", "main")
        .message_type("data")
        .add_value("name", "example")
        .add_value("count", 42)
        .add_value("ratio", 3.14)
        .build();

    // Serialize to binary (high-performance)
    std::string binary_data = container->serialize();

    // Serialize to JSON (human-readable)
    std::string json_data = container->to_json();

    // Deserialize from binary
    auto restored = std::make_shared<value_container>(binary_data);

    // Access values
    auto name = restored->get_value("name");
    auto count = restored->get_value("count");

    return 0;
}
```

### Using Value Factory

```cpp
#include <container/core/container.h>

using namespace container_module;

int main() {
    // Create a container
    auto container = std::make_shared<value_container>();

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

    // Serialize
    std::string data = container->serialize();

    return 0;
}
```

## CMake Integration

### Option 1: Add as Subdirectory

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)
```

### Option 2: Using FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)
```

## Next Steps

- [Features](../FEATURES.md) - Explore all features
- [API Reference](../API_REFERENCE.md) - Complete API documentation
- [Architecture](../ARCHITECTURE.md) - System design and patterns
- [Benchmarks](../BENCHMARKS.md) - Performance analysis
- [FAQ](FAQ.md) - Frequently asked questions

## Troubleshooting

If you encounter issues:

1. **common_system not found**: Ensure common_system is cloned in the same parent directory
2. **C++20 features not available**: Update your compiler to a version supporting C++20 Concepts
3. **CMake version too old**: Update CMake to 3.20 or higher

For more details, see the [Troubleshooting Guide](TROUBLESHOOTING.md).
