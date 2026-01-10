# container_system API Reference

> **Version**: 0.2.1
> **Last Updated**: 2026-01-10
> **Status**: C++20 Concepts integrated, migrating to variant_value_v2 (Phase 2 in progress)

## Table of Contents

1. [Namespace](#namespace)
2. [C++20 Concepts](#c20-concepts)
3. [variant_value_v2 (Recommended)](#variant_value_v2-recommended)
4. [Container](#container)
5. [Async API (C++20 Coroutines)](#async-api-c20-coroutines)
6. [Error Codes](#error-codes)
7. [Serialization/Deserialization](#serializationdeserialization)

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

## Async API (C++20 Coroutines)

### Overview

**Header**: `#include <container/internal/async/async.h>`

**Namespace**: `container_module::async`

**Description**: C++20 coroutine-based asynchronous API for non-blocking container operations.

**Requirements**:
- C++20 compiler with coroutine support
- GCC 10+ (full support in 11+), Clang 13+, MSVC 2019 16.8+
- CMake option: `CONTAINER_ENABLE_COROUTINES=ON` (default)

### task<T>

A lazy coroutine task type representing an asynchronous computation.

#### Constructor

```cpp
task();                         // Default constructor (invalid task)
task(task&& other) noexcept;   // Move constructor
```

#### Methods

##### `valid()`

```cpp
[[nodiscard]] bool valid() const noexcept;
```

**Description**: Check if task holds a valid coroutine handle.

##### `done()`

```cpp
[[nodiscard]] bool done() const noexcept;
```

**Description**: Check if the coroutine has completed.

##### `get()`

```cpp
T get();                    // For task<T>
void get();                 // For task<void>
```

**Description**: Get the result of the task. Rethrows any captured exception.

**Exceptions**: Rethrows any exception that occurred during coroutine execution.

#### Factory Functions

##### `make_ready_task()`

```cpp
template<typename T>
task<T> make_ready_task(T value);

task<void> make_ready_task();
```

**Description**: Create an immediately-ready task with a value.

##### `make_exceptional_task()`

```cpp
template<typename T>
task<T> make_exceptional_task(std::exception_ptr ex);
```

**Description**: Create a task that will rethrow the given exception.

#### Example

```cpp
#include <container/internal/async/async.h>
using namespace container_module::async;

task<int> compute() {
    co_return 42;
}

task<int> chained() {
    int value = co_await compute();
    co_return value * 2;
}

// Usage
auto t = chained();
while (!t.done()) {
    std::this_thread::sleep_for(1ms);
}
int result = t.get();  // 84
```

---

### generator<T>

A coroutine generator for lazily producing sequences of values.

#### Methods

##### `valid()`

```cpp
[[nodiscard]] bool valid() const noexcept;
```

**Description**: Check if generator holds a valid coroutine handle.

##### `begin()` / `end()`

```cpp
iterator begin();
std::default_sentinel_t end();
```

**Description**: Range-based for loop support.

#### Utility Functions

##### `take()`

```cpp
template<typename T>
generator<T> take(generator<T> gen, size_t count);
```

**Description**: Limit a generator to produce at most `count` values.

#### Example

```cpp
generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

// Usage
for (int value : range(0, 10)) {
    std::cout << value << "\n";
}

// With take() for infinite sequences
generator<int> infinite() {
    int i = 0;
    while (true) co_yield i++;
}

for (int value : take(infinite(), 5)) {
    process(value);
}
```

---

### async_container

Async wrapper for `value_container` with coroutine-based operations.

#### Constructors

```cpp
async_container();                                          // Empty container
explicit async_container(std::shared_ptr<value_container> container);  // Wrap existing
async_container(async_container&& other) noexcept;         // Move constructor
```

#### Container Access

##### `get_container()`

```cpp
[[nodiscard]] std::shared_ptr<value_container> get_container() const noexcept;
```

**Description**: Get the underlying container.

##### `set_container()`

```cpp
void set_container(std::shared_ptr<value_container> container) noexcept;
```

**Description**: Replace the underlying container.

#### Value Operations

##### `set()`

```cpp
template<typename T>
async_container& set(std::string_view key, T&& value);
```

**Description**: Set a value in the container. Returns `*this` for chaining.

##### `get()`

```cpp
template<typename T>
[[nodiscard]] std::optional<T> get(std::string_view key) const;
```

**Description**: Get a typed value from the container.

##### `contains()`

```cpp
[[nodiscard]] bool contains(std::string_view key) const noexcept;
```

**Description**: Check if key exists.

#### Async Serialization

##### `serialize_async()`

```cpp
[[nodiscard]] task<Result<std::vector<uint8_t>>> serialize_async() const;
```

**Description**: Serialize container to byte array asynchronously.

##### `serialize_string_async()`

```cpp
[[nodiscard]] task<Result<std::string>> serialize_string_async() const;
```

**Description**: Serialize container to string asynchronously.

#### Async Deserialization

##### `deserialize_async()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_async(std::span<const uint8_t> data);
```

**Description**: Deserialize from byte array asynchronously.

##### `deserialize_string_async()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_string_async(std::string_view data);
```

**Description**: Deserialize from string asynchronously.

#### Async File I/O

##### `load_async()`

```cpp
[[nodiscard]] task<VoidResult> load_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**Description**: Load container from file asynchronously.

**Parameters**:
- `path`: File path to load from
- `callback`: Optional progress callback `void(size_t bytes, size_t total)`

##### `save_async()`

```cpp
[[nodiscard]] task<VoidResult> save_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**Description**: Save container to file asynchronously.

#### Streaming

##### `serialize_chunked()`

```cpp
[[nodiscard]] generator<std::vector<uint8_t>> serialize_chunked(
    size_t chunk_size = 64 * 1024) const;
```

**Description**: Serialize in chunks using a generator.

##### `deserialize_streaming()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_streaming(std::span<const uint8_t> data, bool is_final = true);
```

**Description**: Deserialize with streaming support.

#### Example

```cpp
#include <container/internal/async/async.h>
using namespace container_module::async;

task<void> example() {
    // Create and populate
    async_container cont;
    cont.set("name", std::string("test"))
        .set("value", static_cast<int64_t>(42));

    // Async save with progress
    auto save_result = co_await cont.save_async("data.bin",
        [](size_t bytes, size_t total) {
            std::cout << bytes << "/" << total << "\n";
        });

    if (!save_result.is_ok()) {
        std::cerr << "Save failed: " << save_result.error().message << "\n";
        co_return;
    }

    // Async load
    async_container loaded;
    auto load_result = co_await loaded.load_async("data.bin");

    if (load_result.is_ok()) {
        auto name = loaded.get<std::string>("name");
        // Use loaded data
    }
}
```

---

### Standalone File Utilities

#### `read_file_async()`

```cpp
[[nodiscard]] task<Result<std::vector<uint8_t>>> read_file_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**Description**: Read file contents asynchronously.

#### `write_file_async()`

```cpp
[[nodiscard]] task<VoidResult> write_file_async(
    std::string_view path,
    std::span<const uint8_t> data,
    progress_callback callback = nullptr);
```

**Description**: Write data to file asynchronously.

#### Example

```cpp
task<void> file_example() {
    // Read
    auto read_result = co_await read_file_async("input.bin");
    if (read_result.is_ok()) {
        auto& data = read_result.value();
        // Process data
    }

    // Write
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto write_result = co_await write_file_async("output.bin", data);
}
```

---

### thread_pool_executor

**Header**: `#include <container/internal/async/thread_pool_executor.h>`

Thread pool for efficient async task execution.

#### Constructor

```cpp
explicit thread_pool_executor(size_t thread_count = std::thread::hardware_concurrency());
```

**Description**: Create executor with specified thread count.

#### Methods

##### `submit()`

```cpp
template<typename F>
auto submit(F&& func) -> std::future<std::invoke_result_t<F>>;
```

**Description**: Submit work to the thread pool.

##### `spawn()`

```cpp
template<typename T>
void spawn(task<T> t);
```

**Description**: Spawn a coroutine task on the thread pool.

##### `stop()`

```cpp
void stop();
```

**Description**: Stop the executor and wait for all tasks.

#### Example

```cpp
#include <container/internal/async/thread_pool_executor.h>
using namespace container_module::async;

thread_pool_executor executor(4);

// Submit lambda
auto future = executor.submit([]() {
    return expensive_computation();
});
auto result = future.get();

// Spawn coroutine
task<int> async_work() {
    co_return 42;
}
executor.spawn(async_work());
```

---

### Feature Detection

```cpp
namespace container_module::async {
    inline constexpr bool has_coroutine_support = /* true if coroutines available */;
}
```

**Usage**:
```cpp
#if CONTAINER_HAS_COROUTINES
    // Use async API
#else
    // Fallback to sync API
#endif
```

---

## Error Codes

### Overview

**Header**: `#include <container/core/container/error_codes.h>`

**Description**: Standardized error codes for container_system Result<T> pattern. Error codes are organized into categories by hundreds digit for easy identification and handling.

**Namespace**: `container_module::error_codes`

### Error Code Categories

| Category | Range | Description |
|----------|-------|-------------|
| Value operations | 1xx (100-199) | Key/value access and modification errors |
| Serialization | 2xx (200-299) | Data format and encoding errors |
| Validation | 3xx (300-399) | Schema and constraint validation errors |
| Resource | 4xx (400-499) | File and memory resource errors |
| Thread safety | 5xx (500-599) | Concurrency and locking errors |

### Error Codes Reference

#### Value Operations (1xx)

| Code | Name | Description |
|------|------|-------------|
| 100 | `key_not_found` | Requested key does not exist in the container |
| 101 | `type_mismatch` | Value type does not match the requested type |
| 102 | `value_out_of_range` | Numeric value is outside the valid range |
| 103 | `invalid_value` | Value is invalid for the operation |
| 104 | `key_already_exists` | Key already exists when unique key required |
| 105 | `empty_key` | Empty key name provided |

#### Serialization (2xx)

| Code | Name | Description |
|------|------|-------------|
| 200 | `serialization_failed` | Serialization operation failed |
| 201 | `deserialization_failed` | Deserialization operation failed |
| 202 | `invalid_format` | Data format is invalid or unrecognized |
| 203 | `version_mismatch` | Data version does not match expected version |
| 204 | `corrupted_data` | Data is corrupted or incomplete |
| 205 | `header_parse_failed` | Header parsing failed |
| 206 | `value_parse_failed` | Value parsing failed |
| 207 | `encoding_error` | Encoding/decoding error (e.g., invalid UTF-8) |

#### Validation (3xx)

| Code | Name | Description |
|------|------|-------------|
| 300 | `schema_validation_failed` | Schema validation failed |
| 301 | `missing_required_field` | Required field is missing |
| 302 | `constraint_violated` | Constraint was violated |
| 303 | `type_constraint_violated` | Type constraint not satisfied |
| 304 | `max_size_exceeded` | Maximum size exceeded |

#### Resource (4xx)

| Code | Name | Description |
|------|------|-------------|
| 400 | `memory_allocation_failed` | Memory allocation failed |
| 401 | `file_not_found` | File not found |
| 402 | `file_read_error` | File read error |
| 403 | `file_write_error` | File write error |
| 404 | `permission_denied` | Permission denied |
| 405 | `resource_exhausted` | Resource exhausted |
| 406 | `io_error` | I/O operation failed |

#### Thread Safety (5xx)

| Code | Name | Description |
|------|------|-------------|
| 500 | `lock_acquisition_failed` | Lock acquisition failed |
| 501 | `concurrent_modification` | Concurrent modification detected |
| 502 | `lock_timeout` | Deadlock detected or timeout |

### Utility Functions

#### `get_message(int code)`

Returns a human-readable message for an error code.

```cpp
#include <container/core/container/error_codes.h>

using namespace container_module::error_codes;

auto msg = get_message(key_not_found);  // Returns "Key not found"
auto msg2 = get_message(999);           // Returns "Unknown error"
```

#### `get_category(int code)`

Returns the category name for an error code.

```cpp
auto cat = get_category(100);   // Returns "value_operation"
auto cat2 = get_category(200);  // Returns "serialization"
auto cat3 = get_category(300);  // Returns "validation"
auto cat4 = get_category(400);  // Returns "resource"
auto cat5 = get_category(500);  // Returns "thread_safety"
```

#### Category Check Functions

```cpp
// Check if error belongs to a specific category
bool is_value_error(int code);         // Check if 1xx range
bool is_serialization_error(int code); // Check if 2xx range
bool is_validation_error(int code);    // Check if 3xx range
bool is_resource_error(int code);      // Check if 4xx range
bool is_thread_error(int code);        // Check if 5xx range

// Example usage
if (is_resource_error(err.code)) {
    // Handle resource-related errors
}
```

#### `make_message(int code, std::string_view detail = "")`

Builds a detailed error message with context.

```cpp
auto msg = make_message(file_not_found, "/path/to/file.txt");
// Returns "File not found: /path/to/file.txt"
```

### Usage Example

```cpp
#include <container/core/container/error_codes.h>

using namespace container_module;
using namespace container_module::error_codes;

Result<int> get_value(const value_container& c, std::string_view key) {
    auto val = c.get_value(key);
    if (!val) {
        return error(error_info{
            key_not_found,
            make_message(key_not_found, key),
            "container_system"
        });
    }
    // Process value...
}

// Error handling
auto result = get_value(container, "user_id");
if (!result) {
    const auto& err = result.error();
    if (is_value_error(err.code)) {
        // Handle value-related errors
    } else if (is_resource_error(err.code)) {
        // Handle resource-related errors
    }
}
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
