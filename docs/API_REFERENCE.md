# container_system API Reference

> **Version**: 0.2.1
> **Last Updated**: 2025-12-10
> **Status**: C++20 Concepts integrated, migrating to variant_value_v2 (Phase 2 in progress)

## Table of Contents

1. [Namespace](#namespace)
2. [C++20 Concepts](#c20-concepts)
3. [variant_value_v2 (Recommended)](#variant_value_v2-recommended)
4. [Container](#container)
5. [Serialization/Deserialization](#serializationdeserialization)

---

## Namespace

### `container_module`

All public APIs of container_system are contained in this namespace.

**Sub-namespaces**:
- `container_module::concepts` - C++20 concepts for type validation

**Included Items**:
- `variant_value_v2` - Next-generation Value system (recommended)
- `container` - Value container
- Serialization functions
- C++20 concepts for compile-time type validation

---

## C++20 Concepts

### Overview

**Header**: `#include <container/core/concepts.h>`

**Description**: C++20 concepts for container_system type validation. These concepts enable compile-time validation with clear error messages, replacing SFINAE-based constraints.

**Requirements**:
- C++20 compiler with concepts support
- GCC 10+, Clang 10+, MSVC 2022+

### Type Constraint Concepts

#### `Arithmetic`

```cpp
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;
```

**Description**: A type that is arithmetic (integral or floating-point).

**Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<Arithmetic T>
value make_numeric_value(std::string_view name, T val);

// Usage
auto v1 = make_numeric_value("count", 42);      // OK: int is arithmetic
auto v2 = make_numeric_value("rate", 3.14);     // OK: double is arithmetic
// auto v3 = make_numeric_value("name", "test"); // Error: const char* is not arithmetic
```

#### `IntegralType`

```cpp
template<typename T>
concept IntegralType = std::integral<T>;
```

**Description**: A type that is an integral type.

#### `FloatingPointType`

```cpp
template<typename T>
concept FloatingPointType = std::floating_point<T>;
```

**Description**: A type that is a floating-point type.

#### `SignedIntegral`

```cpp
template<typename T>
concept SignedIntegral = std::signed_integral<T>;
```

**Description**: A signed integral type.

#### `UnsignedIntegral`

```cpp
template<typename T>
concept UnsignedIntegral = std::unsigned_integral<T>;
```

**Description**: An unsigned integral type.

#### `TriviallyCopyable`

```cpp
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
```

**Description**: A type that can be safely copied with memcpy. Use this concept for types that require SIMD-optimized operations or deterministic memory layout.

**Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

// Used in typed_container for SIMD-friendly batches
template<TriviallyCopyable TValue>
class typed_container {
    std::vector<TValue> values_;
public:
    void push(const TValue& value) { values_.push_back(value); }
};

// Usage
typed_container<int> int_container;       // OK: int is trivially copyable
typed_container<double> double_container; // OK: double is trivially copyable
// typed_container<std::string> str_container; // Error: std::string is not trivially copyable
```

### Value Type Concepts

#### `ValueVariantType`

```cpp
template<typename T>
concept ValueVariantType =
    std::same_as<std::decay_t<T>, std::monostate> ||
    std::same_as<std::decay_t<T>, bool> ||
    std::same_as<std::decay_t<T>, int16_t> ||
    std::same_as<std::decay_t<T>, uint16_t> ||
    std::same_as<std::decay_t<T>, int32_t> ||
    std::same_as<std::decay_t<T>, uint32_t> ||
    std::same_as<std::decay_t<T>, int64_t> ||
    std::same_as<std::decay_t<T>, uint64_t> ||
    std::same_as<std::decay_t<T>, float> ||
    std::same_as<std::decay_t<T>, double> ||
    std::same_as<std::decay_t<T>, std::string> ||
    std::same_as<std::decay_t<T>, std::vector<uint8_t>> ||
    std::same_as<std::decay_t<T>, std::shared_ptr<thread_safe_container>> ||
    std::same_as<std::decay_t<T>, array_variant>;
```

**Description**: A type that is valid for storage in ValueVariant.

**Valid types**:
- `std::monostate` (null)
- `bool`
- `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- `float`, `double`
- `std::string`
- `std::vector<uint8_t>` (bytes)
- `std::shared_ptr<thread_safe_container>`
- `array_variant`

**Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<ValueVariantType T>
void set_typed(std::string_view key, T&& val);

// Usage
set_typed("count", int32_t{42});      // OK
set_typed("name", std::string{"test"}); // OK
set_typed("enabled", true);            // OK
```

#### `NumericValueType`

```cpp
template<typename T>
concept NumericValueType = Arithmetic<T> && !std::same_as<std::decay_t<T>, bool>;
```

**Description**: A numeric type suitable for value storage. Excludes bool (which has its own handling) and includes all signed/unsigned integers and floating-point types.

#### `StringLike`

```cpp
template<typename T>
concept StringLike =
    std::same_as<std::decay_t<T>, std::string> ||
    std::same_as<std::decay_t<T>, std::string_view> ||
    std::same_as<std::decay_t<T>, const char*> ||
    std::convertible_to<T, std::string_view>;
```

**Description**: A type that can be converted to or used as a string.

#### `ByteContainer`

```cpp
template<typename T>
concept ByteContainer = std::same_as<std::decay_t<T>, std::vector<uint8_t>>;
```

**Description**: A type that represents byte data.

### Callback Concepts

#### `ValueVisitor`

```cpp
template<typename V>
concept ValueVisitor = std::move_constructible<V>;
```

**Description**: A callable type that can visit value variants.

**Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<ValueVisitor V>
auto visit(V&& visitor) const;

// Usage with lambda
value val;
val.visit([](const auto& v) {
    std::cout << v << std::endl;
});
```

#### `KeyValueCallback`

```cpp
template<typename F>
concept KeyValueCallback = std::invocable<F, const std::string&, const value&>;
```

**Description**: A callable for const key-value pair iteration.

**Example**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<KeyValueCallback Func>
void for_each(Func&& func) const;

// Usage
container.for_each([](const std::string& key, const value& val) {
    std::cout << key << ": " << val.to_string() << std::endl;
});
```

#### `MutableKeyValueCallback`

```cpp
template<typename F>
concept MutableKeyValueCallback = std::invocable<F, const std::string&, value&>;
```

**Description**: A callable for mutable key-value pair iteration.

**Example**:
```cpp
template<MutableKeyValueCallback Func>
void for_each_mut(Func&& func);

// Usage - modify values in place
container.for_each_mut([](const std::string& key, value& val) {
    // Modify val
});
```

#### `ValueMapCallback`

```cpp
template<typename F, typename Map>
concept ValueMapCallback = std::invocable<F, Map&>;
```

**Description**: A callable that operates on the entire value map. Used for bulk update operations.

#### `ConstValueMapCallback`

```cpp
template<typename F, typename Map>
concept ConstValueMapCallback = std::invocable<F, const Map&>;
```

**Description**: A callable that operates on a const value map. Used for bulk read operations.

### Serialization Concepts

#### `Serializable`

```cpp
template<typename T>
concept Serializable = requires(const T& t) {
    { t.serialize() } -> std::convertible_to<std::vector<uint8_t>>;
};
```

**Description**: A type that provides binary serialization methods.

#### `JsonSerializable`

```cpp
template<typename T>
concept JsonSerializable = requires(const T& t) {
    { t.to_json() } -> std::convertible_to<std::string>;
};
```

**Description**: A type that can be serialized to JSON.

### Container Concepts

#### `ContainerValue`

```cpp
template<typename T>
concept ContainerValue =
    std::same_as<std::decay_t<T>, std::shared_ptr<thread_safe_container>>;
```

**Description**: A type representing a nested container value. Used for hierarchical data structures.

### Concepts Summary Table

| Concept | Description | Usage |
|---------|-------------|-------|
| `Arithmetic` | Integral or floating-point types | Template constraints for numeric value creation |
| `IntegralType` | Integral types | Type checking in messaging integration |
| `FloatingPointType` | Floating-point types | Type checking in messaging integration |
| `SignedIntegral` | Signed integral types | Numeric validation |
| `UnsignedIntegral` | Unsigned integral types | Numeric validation |
| `TriviallyCopyable` | Types safe for memcpy/SIMD | `typed_container` template constraint |
| `ValueVariantType` | Types valid for `ValueVariant` | Type-safe value operations |
| `NumericValueType` | Numeric types excluding bool | Value factory operations |
| `StringLike` | String-convertible types | String value handling |
| `ByteContainer` | Byte data containers | Binary data handling |
| `ValueVisitor` | Callable for visiting values | Visitor pattern support |
| `KeyValueCallback` | Callable for const iteration | `for_each()` function |
| `MutableKeyValueCallback` | Callable for mutable iteration | `for_each_mut()` function |
| `ValueMapCallback` | Callable for bulk map updates | `bulk_update()` function |
| `ConstValueMapCallback` | Callable for const bulk reads | `bulk_read()` function |
| `Serializable` | Types with serialization | Serialization support |
| `JsonSerializable` | Types with JSON serialization | JSON output support |
| `ContainerValue` | Nested container types | Hierarchical data structures |

---

## variant_value_v2 (Recommended)

### Overview

**Header**: `#include <container/values/variant_value_v2.h>`

**Description**: High-performance type-safe variant value implementation (4.39x performance improvement)

**Supported Types**:
- Primitive types: `bool`, `int`, `uint`, `long`, `float`, `double`, `string`
- Container types: `vector`, `map`
- Binary types: `bytes`

### Creation and Access

```cpp
#include <container/values/variant_value_v2.h>

using namespace container_module;

// Creation
variant_value_v2 val_int(42);
variant_value_v2 val_str("hello");
variant_value_v2 val_double(3.14);

// Type checking
if (val_int.is<int>()) {
    // int type
}

// Value access
int i = val_int.get<int>();                    // 42
std::string s = val_str.get<std::string>();    // "hello"
```

### Container Types

```cpp
// Vector
variant_value_v2 vec;
vec.set<std::vector<variant_value_v2>>({
    variant_value_v2(1),
    variant_value_v2(2),
    variant_value_v2(3)
});

auto& v = vec.get<std::vector<variant_value_v2>>();
for (const auto& item : v) {
    std::cout << item.get<int>() << std::endl;
}

// Map
variant_value_v2 map;
std::map<std::string, variant_value_v2> m;
m["name"] = variant_value_v2("Alice");
m["age"] = variant_value_v2(30);
map.set(m);
```

### Core Methods

#### `is<T>()`

```cpp
template <typename T>
bool is() const;
```

**Description**: Type checking

**Example**:
```cpp
variant_value_v2 val(42);
if (val.is<int>()) {
    // int type
}
```

#### `get<T>()`

```cpp
template <typename T>
T& get();

template <typename T>
const T& get() const;
```

**Description**: Value access

**Exceptions**:
- `std::bad_variant_access`: Thrown on type mismatch

#### `set<T>()`

```cpp
template <typename T>
void set(const T& value);
```

**Description**: Set value

---

## Container

### container class

**Header**: `#include <container/container.h>`

**Description**: Key-Value store

#### Creation and Usage

```cpp
#include <container/container.h>

using namespace container_module;

container c;

// Add values
c.add("name", variant_value_v2("Alice"));
c.add("age", variant_value_v2(30));
c.add("score", variant_value_v2(95.5));

// Query values
if (c.contains("name")) {
    auto name = c.get("name").get<std::string>();
    std::cout << "Name: " << name << std::endl;
}

// Remove values
c.remove("score");
```

#### Core Methods

##### `add()`

```cpp
void add(const std::string& key, const variant_value_v2& value);
```

**Description**: Add Key-Value pair

##### `get()`

```cpp
variant_value_v2& get(const std::string& key);
const variant_value_v2& get(const std::string& key) const;
```

**Description**: Query value

**Exceptions**:
- `std::out_of_range`: Thrown when key does not exist

##### `contains()`

```cpp
bool contains(const std::string& key) const;
```

**Description**: Check if key exists

##### `remove()`

```cpp
bool remove(const std::string& key);
```

**Description**: Remove Key-Value pair

**Return Value**:
- `true`: Successfully removed
- `false`: Key does not exist

---

## Metrics

### Overview

**Header**: `#include <container/core/container/metrics.h>`

**Description**: Comprehensive observability metrics infrastructure for monitoring container operations with nanosecond precision timing, latency histograms, and multiple export formats.

### Metrics Structures

#### `operation_counts`

```cpp
struct operation_counts {
    std::atomic<uint64_t> reads{0};
    std::atomic<uint64_t> writes{0};
    std::atomic<uint64_t> serializations{0};
    std::atomic<uint64_t> deserializations{0};
    std::atomic<uint64_t> copies{0};
    std::atomic<uint64_t> moves{0};
};
```

**Description**: Thread-safe atomic counters for all container operations.

#### `timing_metrics`

```cpp
struct timing_metrics {
    std::atomic<uint64_t> total_serialize_ns{0};
    std::atomic<uint64_t> total_deserialize_ns{0};
    std::atomic<uint64_t> total_read_ns{0};
    std::atomic<uint64_t> total_write_ns{0};
};
```

**Description**: Cumulative timing in nanoseconds for each operation type.

#### `latency_histogram`

```cpp
struct latency_histogram {
    static constexpr size_t kReservoirSize = 1024;

    void record(uint64_t latency_ns) noexcept;
    uint64_t get_percentile(double percentile) const noexcept;
    uint64_t p50() const noexcept;
    uint64_t p95() const noexcept;
    uint64_t p99() const noexcept;
    uint64_t p999() const noexcept;
    void reset() noexcept;
};
```

**Description**: Latency histogram with reservoir sampling (1024 samples) for percentile calculations.

**Example**:
```cpp
auto metrics = value_container::get_detailed_metrics();
std::cout << "P50 serialize: " << metrics.serialize_latency.p50() << " ns\n";
std::cout << "P99 serialize: " << metrics.serialize_latency.p99() << " ns\n";
```

### Static Methods (value_container)

#### `get_detailed_metrics()`

```cpp
static detailed_metrics get_detailed_metrics();
```

**Description**: Returns a copy of current metrics state.

#### `reset_metrics()`

```cpp
static void reset_metrics();
```

**Description**: Resets all metrics to zero.

#### `set_metrics_enabled()` / `is_metrics_enabled()`

```cpp
static void set_metrics_enabled(bool enabled);
static bool is_metrics_enabled();
```

**Description**: Enable/disable metrics collection. When disabled, metrics operations have zero overhead.

### Instance Methods (value_container)

#### `metrics_to_json()`

```cpp
std::string metrics_to_json() const;
```

**Description**: Export metrics in JSON format (Grafana/ElasticSearch compatible).

**Example**:
```cpp
value_container c;
c.set_metrics_enabled(true);
// ... perform operations ...
std::string json = c.metrics_to_json();
```

#### `metrics_to_prometheus()`

```cpp
std::string metrics_to_prometheus() const;
```

**Description**: Export metrics in Prometheus exposition format.

**Example**:
```cpp
value_container c;
std::string prometheus = c.metrics_to_prometheus();
// Output: container_operations_total{type="reads"} 42
```

### Usage Example

```cpp
#include <container/container.h>
using namespace container_module;

// Enable metrics
value_container::set_metrics_enabled(true);

// Perform operations
value_container c;
c.set("key1", "value1");
c.set("key2", 42);
auto serialized = c.serialize();
value_container c2;
c2.deserialize(serialized);

// Get metrics
auto metrics = value_container::get_detailed_metrics();
std::cout << "Reads: " << metrics.operations.reads.load() << "\n";
std::cout << "Writes: " << metrics.operations.writes.load() << "\n";
std::cout << "Serializations: " << metrics.operations.serializations.load() << "\n";
std::cout << "P99 serialize latency: " << metrics.serialize_latency.p99() << " ns\n";

// Export for monitoring
std::string json = c.metrics_to_json();
std::string prometheus = c.metrics_to_prometheus();

// Reset metrics
value_container::reset_metrics();

// Disable for production
value_container::set_metrics_enabled(false);
```

---

## Serialization/Deserialization

### JSON Serialization

```cpp
#include <container/serialization.h>

using namespace container_module;

container c;
c.add("name", variant_value_v2("Alice"));
c.add("age", variant_value_v2(30));

// Serialize to JSON
std::string json = serialize_to_json(c);
// {"name":"Alice","age":30}

// Deserialize from JSON
container c2 = deserialize_from_json(json);
```

### Binary Serialization

```cpp
// Serialize to binary
std::vector<uint8_t> binary = serialize_to_binary(c);

// Deserialize from binary
container c2 = deserialize_from_binary(binary);
```

---

## Performance

### variant_value_v2 Benchmarks

| Operation | v1 (legacy) | v2 (new) | Improvement |
|-----------|-------------|----------|-------------|
| Creation | 45 ns | 10 ns | 4.5x |
| Copy | 52 ns | 12 ns | 4.33x |
| Access | 8 ns | 2 ns | 4.0x |
| Type check | 5 ns | 1 ns | 5.0x |

**Overall Average**: 4.39x performance improvement

**Test Environment**: Apple M1 Max, 10 cores, macOS 14

---

## Migration Guide

### From legacy value to variant_value_v2

**v1 (legacy)**:
```cpp
value val_old;
val_old.set_int(42);
int i = val_old.get_int();
```

**v2 (new, recommended)**:
```cpp
variant_value_v2 val_new(42);
int i = val_new.get<int>();
```

**Benefits**:
- Type safety (compile-time checking)
- Performance (4.39x improvement)
- Modern C++ API

**Detailed Guide**: [MIGRATION_GUIDE.md](advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md)

---

## Notes

### Migration in Progress

- **Phase 1**: ✅ Complete (2025-11-06)
  - variant_value_v2 implementation
  - 19/19 tests passing
- **C++20 Concepts**: ✅ Complete (2025-12-09)
  - Integrated C++20 concepts for type validation
  - Replaces SFINAE-based constraints with clear error messages
  - Added 18 concepts in `core/concepts.h`
- **Phase 2**: 🔄 In Progress
  - Core container migration
  - Factory function implementation
- **Phase 3-5**: ⏳ Pending

### Recommendations

- **New code**: Use variant_value_v2 (recommended)
- **Existing code**: Gradual migration (see MIGRATION_GUIDE)
- **Type constraints**: Use C++20 concepts for compile-time validation

---

**Created**: 2025-11-21
**Updated**: 2025-12-10
**Version**: 0.2.1
**Author**: kcenon@naver.com
