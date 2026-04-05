# Getting Started with container_system

A step-by-step guide to using `container_system` -- from installation through
serialization formats, async coroutines, and messaging integration.

## Prerequisites

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| C++ standard | C++20 | C++20 |
| CMake | 3.20 | 3.28+ |
| GCC | 11 | 13+ |
| Clang | 14 | 16+ |
| Apple Clang | 14 | 15+ |
| MSVC | 2022 (17.0) | 2022 (17.8+) |

**Required dependency**: [common_system](https://github.com/kcenon/common_system)
must be available to CMake (via `FetchContent`, `find_package`, or a sibling
directory).

## Installation

### Option A -- CMake FetchContent (recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(container_system)

target_link_libraries(your_target PRIVATE container_system)
```

### Option B -- Clone and build locally

```bash
git clone https://github.com/kcenon/container_system.git
cd container_system

# Build with the helper script
./scripts/build.sh

# Or use CMake directly
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Option C -- Add as a Git submodule

```bash
git submodule add https://github.com/kcenon/container_system.git external/container_system
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(external/container_system)
target_link_libraries(your_target PRIVATE container_system)
```

## Your First Container

The primary class is `value_container` (aliased as `message_buffer` since v2.0).
Create one, set metadata, and store typed values:

```cpp
#include "container.h"

using namespace kcenon::container;

int main() {
    // Create a container
    auto container = std::make_shared<value_container>();

    // Set routing metadata
    container->set_source("my_service", "session_001");
    container->set_target("backend", "main_handler");
    container->set_message_type("user_profile");

    // Store typed values
    container->set("username", std::string("alice"));
    container->set("user_id",  static_cast<int32_t>(42));
    container->set("balance",  1500.75);
    container->set("active",   true);

    // Read values back
    if (auto val = container->get("username")) {
        if (auto* s = std::get_if<std::string>(&val->data)) {
            std::cout << "Username: " << *s << std::endl;
        }
    }

    return 0;
}
```

> **Tip**: `set()` returns `*this`, so you can chain calls:
> `container->set("a", 1).set("b", 2).set("c", 3);`

## Value Types

`container_system` supports 16 value types through the `value_types` enum and
the `value_variant` (`std::variant`). The most common types and their C++ mappings:

| value_types | C++ Type | Example |
|-------------|----------|---------|
| `null_value` | `std::monostate` | (default) |
| `bool_value` | `bool` | `true` |
| `short_value` | `int16_t` | `static_cast<int16_t>(100)` |
| `int_value` | `int32_t` | `static_cast<int32_t>(42)` |
| `long_value` | `int64_t` | `static_cast<int64_t>(9876543210)` |
| `float_value` | `float` | `3.14f` |
| `double_value` | `double` | `2.71828` |
| `string_value` | `std::string` | `std::string("hello")` |
| `bytes_value` | `std::vector<uint8_t>` | raw byte data |

Additional types include `ushort_value`, `uint_value`, `ulong_value`,
`llong_value`, `ullong_value`, `container_value` (nested containers),
and `array_value`.

### Reading values with get()

```cpp
// Returns std::optional<optimized_value>
if (auto val = container->get("score")) {
    if (auto* d = std::get_if<double>(&val->data)) {
        std::cout << "Score: " << *d << std::endl;
    }
}
```

### Checking existence and iteration

```cpp
// Check if a key exists
if (container->contains("username")) { /* ... */ }

// Range-based for loop over all values
for (const auto& val : *container) {
    std::cout << val.name << " (type index: "
              << static_cast<int>(val.type) << ")" << std::endl;
}
```

## Serialization

`container_system` supports four serialization formats through a unified API.

### Binary format (default, fastest)

```cpp
// Serialize
auto result = container->serialize_string(
    value_container::serialization_format::binary);
std::string data = result.value();

// Deserialize
auto restored = std::make_shared<value_container>(data);
```

### JSON format

```cpp
auto json_result = container->serialize_string(
    value_container::serialization_format::json);
if (json_result.is_ok()) {
    std::cout << json_result.value() << std::endl;
}
```

### MessagePack format

```cpp
auto msgpack_result = container->serialize(
    value_container::serialization_format::msgpack);
```

### Auto-detection

```cpp
// Detect format from raw data
auto format = value_container::detect_format(raw_bytes);

// Or let deserialize() auto-detect
auto result = container->deserialize(raw_data);
```

### Format comparison

| Format | Strength | Use Case |
|--------|----------|----------|
| Binary | Fastest, smallest | Internal IPC, high throughput |
| JSON | Human-readable | Debugging, REST APIs, config |
| XML | Structured, verbose | Legacy interop |
| MessagePack | Compact binary, cross-language | Polyglot services |

## Async Coroutines

When built with `CONTAINER_ENABLE_COROUTINES=ON` (requires C++20), you can use
non-blocking serialization and file I/O through the async API.

```cpp
#include "container.h"
#include "internal/async/async.h"

using namespace kcenon::container;
using namespace kcenon::container::async;

// Async serialization returns task<T>
task<void> process_data() {
    auto container = std::make_shared<value_container>();
    container->set("data", std::string("async payload"));

    async_container async_cont(container);

    // Non-blocking serialize
    auto result = co_await async_cont.serialize_async();

    // Non-blocking deserialize
    auto restored = co_await async_container::deserialize_async(
        result.value());

    co_return;
}
```

### Chunked streaming with generators

For large containers, stream serialized chunks instead of allocating
one large buffer:

```cpp
task<void> stream_large_data() {
    auto container = std::make_shared<value_container>();
    // ... populate with large data ...

    async_container async_cont(container);

    // Stream in 8 KB chunks
    for (auto chunk : async_cont.serialize_chunked(8 * 1024)) {
        send_over_network(chunk);  // send each chunk as it is produced
    }

    co_return;
}
```

### Async file I/O

```cpp
task<void> file_operations() {
    async_container async_cont(container);

    // Save to disk (non-blocking)
    co_await async_cont.save_async("/tmp/data.bin");

    // Load from disk (non-blocking)
    async_container loaded;
    co_await loaded.load_async("/tmp/data.bin");

    co_return;
}
```

## Messaging Integration

`container_system` sits at **Tier 1** of the kcenon ecosystem, providing the
data exchange format used by `network_system`, `database_system`, and
`pacs_system`.

Every container carries routing metadata that maps naturally to messaging
patterns:

```cpp
auto msg = std::make_shared<value_container>();

// Routing metadata
msg->set_source("order_service", "handler_1");
msg->set_target("inventory_service", "stock_checker");
msg->set_message_type("check_stock");

// Payload
msg->set("product_id", static_cast<int32_t>(12345));
msg->set("quantity",    static_cast<int32_t>(10));

// Serialize for network transmission
auto bytes = msg->serialize(value_container::serialization_format::binary);

// On the receiving end
auto received = std::make_shared<value_container>(bytes.value(), false);
std::cout << "From: " << received->source_id() << std::endl;
std::cout << "Type: " << received->message_type() << std::endl;
```

> **Note**: Since v2.0, `message_buffer` is the preferred alias for
> `value_container`. Both names refer to the same class.

## Next Steps

| Topic | Document |
|-------|----------|
| Full API surface | [API Reference](API_REFERENCE.md) |
| Quick lookup cheat sheet | [API Quick Reference](API_QUICK_REFERENCE.md) |
| Serialization schema policies | [Schema Policy Guide](SERIALIZATION_SCHEMA_POLICY_GUIDE.md) |
| Async coroutine deep dive | [Async Guide](ASYNC_GUIDE.md) |
| Architecture and internals | [Architecture](ARCHITECTURE.md) |
| Performance benchmarks | [Benchmarks](BENCHMARKS.md) |
| Production quality report | [Production Quality](PRODUCTION_QUALITY.md) |
| Running examples | [Examples README](../examples/README.md) |
