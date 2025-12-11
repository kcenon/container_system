# Container System - Frequently Asked Questions

> **Version:** 0.1.0
> **Last Updated:** 2025-11-11
> **Audience:** Users, Developers

This FAQ addresses common questions about the container_system, covering containers, values, serialization, performance, and integration.

---

## Table of Contents

1. [General Questions](#general-questions)
2. [Container Basics](#container-basics)
3. [Value Types](#value-types)
4. [Serialization](#serialization)
5. [Performance](#performance)
6. [Integration](#integration)
7. [Advanced Topics](#advanced-topics)

---

## General Questions

### 1. What is the container_system?

A type-safe, high-performance container and serialization system for C++20:
- **Type-safe containers** using `std::variant`
- **Multiple serialization formats** (Binary, JSON, XML)
- **SIMD optimization** (NEON, AVX2)
- **Memory pooling** for high throughput
- **5M containers/second** creation rate

```cpp
#include <container/container.hpp>

auto container = container::create("user_data");
container->set_value("name", "John");
container->set_value("age", 30);
container->set_value("score", 95.5);

// Serialize to binary
auto binary = container->serialize(format::binary);
```

---

### 2. What C++ standard is required?

**Required:** C++17 (C++20 recommended for concepts)

**Compiler Support:**
- GCC 9+
- Clang 10+
- MSVC 2019+

---

### 3. How does it differ from std::map or JSON libraries?

| Feature | container_system | std::map | JSON libraries |
|---------|------------------|----------|----------------|
| Type Safety | Compile-time | Runtime | Runtime |
| Performance | 5M ops/s | ~2M ops/s | ~500K ops/s |
| Serialization | Built-in | Manual | Built-in |
| SIMD Support | Yes | No | No |
| Memory Pooling | Yes | No | Some |

---

## Container Basics

### 4. How do I create a container?

```cpp
// Empty container
auto container1 = container::create();

// Named container
auto container2 = container::create("user_profile");

// With initial values
auto container3 = container::create("config");
container3->set_value("timeout", 30);
container3->set_value("retry_count", 3);
```

---

### 5. How do I add values to a container?

```cpp
auto c = container::create();

// Primitive types
c->set_value("name", "Alice");           // string
c->set_value("age", 25);                  // int
c->set_value("height", 165.5);            // double
c->set_value("active", true);             // bool

// Arrays
c->set_value("tags", std::vector<std::string>{"cpp", "backend"});
c->set_value("scores", std::vector<int>{85, 90, 95});

// Nested containers
auto address = container::create();
address->set_value("city", "Seoul");
address->set_value("zip", "12345");
c->set_value("address", address);

// Binary data
std::vector<uint8_t> data{0x01, 0x02, 0x03};
c->set_value("binary", data);
```

---

### 6. How do I retrieve values?

```cpp
// Direct retrieval
auto name = c->get_value<std::string>("name");
auto age = c->get_value<int>("age");

// Safe retrieval with default
auto timeout = c->get_value_or("timeout", 30);

// Check if key exists
if (c->contains("email")) {
    auto email = c->get_value<std::string>("email");
}

// Iterate all values
for (const auto& [key, value] : c->get_all_values()) {
    std::cout << key << ": " << value.to_string() << "\n";
}
```

---

## Value Types

### 7. What value types are supported?

**Primitive Types:**
- `string`, `bool`, `int8_t`, `uint8_t`, `int16_t`, `uint16_t`
- `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- `float`, `double`

**Container Types:**
- `array` (homogeneous arrays)
- `container` (nested containers)
- `bytes` (binary data)

**Example:**
```cpp
// All supported types
c->set_value("str", std::string("text"));
c->set_value("i32", int32_t(42));
c->set_value("f64", double(3.14));
c->set_value("arr", std::vector<int>{1,2,3});
c->set_value("bin", std::vector<uint8_t>{0xAB,0xCD});
```

---

### 8. How do I work with arrays?

```cpp
// Create array
std::vector<int> numbers{10, 20, 30, 40, 50};
c->set_value("numbers", numbers);

// Retrieve array
auto retrieved = c->get_value<std::vector<int>>("numbers");

// Array of strings
c->set_value("tags", std::vector<std::string>{"a", "b", "c"});

// Array of containers
std::vector<std::shared_ptr<container>> items;
for (int i = 0; i < 3; ++i) {
    auto item = container::create();
    item->set_value("id", i);
    items.push_back(item);
}
c->set_value("items", items);
```

---

### 9. How do I work with nested containers?

```cpp
// Create nested structure
auto root = container::create("root");

auto user = container::create("user");
user->set_value("name", "Alice");
user->set_value("age", 30);

auto address = container::create("address");
address->set_value("city", "Seoul");
address->set_value("country", "Korea");

user->set_value("address", address);
root->set_value("user", user);

// Access nested values
auto nested_user = root->get_value<std::shared_ptr<container>>("user");
auto nested_address = nested_user->get_value<std::shared_ptr<container>>("address");
auto city = nested_address->get_value<std::string>("city");
```

---

## Serialization

### 10. What serialization formats are supported?

**Binary Format** (fastest):
```cpp
auto binary = c->serialize(format::binary);
auto restored = container::deserialize(binary, format::binary);
```

**JSON Format** (human-readable):
```cpp
auto json = c->serialize(format::json);
auto restored = container::deserialize(json, format::json);
```

**XML Format** (interoperability):
```cpp
auto xml = c->serialize(format::xml);
auto restored = container::deserialize(xml, format::xml);
```

---

### 11. Which format should I use?

| Format | Speed | Size | Use Case |
|--------|-------|------|----------|
| **Binary** | Fastest (2M/s) | Smallest | Network, database storage |
| **JSON** | Medium (500K/s) | Medium | REST APIs, configuration |
| **XML** | Slowest (200K/s) | Largest | Legacy systems, SOAP |

---

### 12. How do I serialize to/from JSON?

```cpp
#include <container/serializers/json_serializer.hpp>

// Serialize
auto c = container::create();
c->set_value("name", "Alice");
c->set_value("age", 30);

auto json_str = c->serialize(format::json);
// Result: {"name":"Alice","age":30}

// Deserialize
auto restored = container::deserialize(json_str, format::json);
auto name = restored->get_value<std::string>("name");
```

---

## Performance

### 13. What is the performance?

**Benchmarks** (3.2 GHz Intel Core i7):

| Operation | Throughput | Latency |
|-----------|------------|---------|
| Container creation | 5M/s | 200 ns |
| Binary serialize | 2M/s | 500 ns |
| JSON serialize | 500K/s | 2 μs |
| Value access | 25M/s | 40 ns |
| SIMD operations | 25M/s | 40 ns |

---

### 14. How do I optimize performance?

```cpp
// 1. Use binary format
auto data = c->serialize(format::binary);  // Fastest

// 2. Reuse containers
container_pool pool(1000);
auto c = pool.acquire();
// ... use container ...
pool.release(c);

// 3. Use SIMD for numeric arrays
std::vector<float> data(1000);
// SIMD-accelerated operations automatically applied

// 4. Preallocate for known size
auto c = container::create();
c->reserve(100);  // Preallocate space for 100 values
```

---

### 15. Does it support SIMD?

Yes, **automatic SIMD** for numeric arrays:

```cpp
// SIMD automatically used on ARM (NEON) and x86 (AVX2)
std::vector<float> data{1.0f, 2.0f, 3.0f, 4.0f};
c->set_value("data", data);

// Optimized operations
auto result = simd::add(data, 10.0f);  // Vectorized addition
auto sum = simd::sum(data);             // Vectorized sum
```

**Performance:**
- **NEON (ARM):** 4-8x faster
- **AVX2 (x86):** 8-16x faster

---

## Integration

### 16. How do I integrate with messaging_system?

```cpp
#include <container/integration/messaging_adapter.hpp>

// Convert to messaging_system format
auto message = messaging_adapter::to_message(container);

// Convert from messaging_system format
auto container = messaging_adapter::from_message(message);
```

---

### 17. How do I integrate with network_system?

```cpp
#include <container/integration/network_adapter.hpp>

// Serialize for network transmission
auto packet = network_adapter::to_packet(container);

// Deserialize from network
auto container = network_adapter::from_packet(packet);
```

---

### 18. How do I integrate with database_system?

```cpp
#include <container/integration/database_adapter.hpp>

// Convert to database row
auto row = database_adapter::to_row(container);

// Convert from database row
auto container = database_adapter::from_row(row);
```

---

## Advanced Topics

### 19. Is it thread-safe?

**Read-only:** Thread-safe
**Modifications:** Use locks or thread-local containers

```cpp
// Option 1: Mutex protection
std::mutex mutex;
{
    std::lock_guard<std::mutex> lock(mutex);
    c->set_value("key", "value");
}

// Option 2: Thread-local containers
thread_local auto c = container::create();
c->set_value("thread_id", std::this_thread::get_id());
```

---

### 20. How do I extend with custom types?

```cpp
// Define custom type
struct CustomData {
    int id;
    std::string name;
};

// Implement serialization
template<>
struct value_serializer<CustomData> {
    static auto serialize(const CustomData& data) -> std::vector<uint8_t> {
        // Serialize logic
    }

    static auto deserialize(const std::vector<uint8_t>& bytes) -> CustomData {
        // Deserialize logic
    }
};

// Use custom type
CustomData data{42, "example"};
c->set_value("custom", data);
```

---

### 21. How do I migrate from messaging_system containers?

```cpp
#include <container/migration/legacy_adapter.hpp>

// Convert legacy container
auto legacy = messaging::container::create();
auto modern = legacy_adapter::convert(legacy);

// Backward compatibility mode
container::enable_legacy_mode();
```

---

### 22. What is the memory overhead?

**Memory Usage:**
- **Empty container:** ~128 bytes
- **Per value:** 24-48 bytes (depends on type)
- **String:** 24 bytes + string length
- **Container:** 128 bytes + nested values

**Optimization:**
```cpp
// Use memory pooling to reduce allocation overhead
container_pool pool(capacity);
auto c1 = pool.acquire();  // No allocation
auto c2 = pool.acquire();  // No allocation
```

---

### 23. Can I use it in embedded systems?

**Yes**, with considerations:

```cpp
// Disable features for embedded
#define CONTAINER_NO_EXCEPTIONS
#define CONTAINER_NO_SIMD
#define CONTAINER_SMALL_MEMORY

// Use static allocation
static_container<256> c;  // 256-byte fixed capacity
```

---

### 24. How do I debug containers?

```cpp
// Enable debug mode
#define CONTAINER_DEBUG

// Print container contents
std::cout << c->to_string() << "\n";

// Validate container
if (!c->validate()) {
    std::cerr << "Container validation failed\n";
}

// Get diagnostics
auto diag = c->get_diagnostics();
std::cout << "Keys: " << diag.key_count << "\n";
std::cout << "Memory: " << diag.memory_usage << " bytes\n";
```

---

### 25. Where can I find more examples?

**Documentation:**
- [Quick Start](QUICK_START.md) - 5-minute guide
- [Best Practices](BEST_PRACTICES.md) - Production patterns
- [Architecture](../ARCHITECTURE.md) - System design
- [Integration](../INTEGRATION.md) - Integration guides

**Examples:**
- `examples/basic/` - Basic usage
- `examples/serialization/` - Serialization formats
- `examples/integration/` - System integration
- `examples/performance/` - Performance optimization

**Support:**
- [GitHub Issues](https://github.com/kcenon/container_system/issues)
- [GitHub Discussions](https://github.com/kcenon/container_system/discussions)

---

**Last Updated:** 2025-11-11
**Next Review:** 2026-02-11
