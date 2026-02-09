# Serialization, Schema, and Policy Framework

**Date**: 2026-02-09
**Status**: Active

## Overview

container_system includes three interconnected subsystems for data serialization,
validation, and configurable storage:

1. **Serialization Framework** — Strategy-based multi-format serializers (JSON, Binary, XML, MessagePack)
2. **Schema Validation** — Runtime container structure validation with fluent API
3. **Policy Container** — Template-based container with pluggable storage strategies

**Namespace**: `container_module`

**Headers**:
- `core/serializers/serializer_strategy.h` — Base interface and format enum
- `core/serializers/serializer_factory.h` — Factory for creating serializers
- `core/serializers/json_serializer.h` — JSON format
- `core/serializers/binary_serializer.h` — Custom binary format
- `core/serializers/xml_serializer.h` — XML format
- `core/serializers/msgpack_serializer.h` — MessagePack format
- `core/container/schema.h` — Schema validation
- `core/policy_container.h` — Policy-based container template
- `core/storage_policy.h` — Storage policy concept and implementations

---

## 1. Serialization Framework

### Architecture

The serialization system uses the **Strategy Pattern**. All format-specific
serializers implement `serializer_strategy`, and `serializer_factory` creates
instances by format.

```
serializer_strategy (abstract base)
├── json_serializer      → serialization_format::json
├── binary_serializer    → serialization_format::binary
├── xml_serializer       → serialization_format::xml
└── msgpack_serializer   → serialization_format::msgpack

serializer_factory::create(format) → unique_ptr<serializer_strategy>
```

### Serialization Formats

| Serializer | Format | Output | Characteristics | Use Case |
|-----------|--------|--------|-----------------|----------|
| `json_serializer` | Text (JSON) | Human-readable | RFC 8259, proper escaping | Debugging, config files, REST APIs |
| `binary_serializer` | Binary | `@header{};@data{};` | Custom format, key-value pairs | Internal transfer, backward compatibility |
| `xml_serializer` | Text (XML) | XML 1.0 | Entity encoding | SOAP, legacy system integration |
| `msgpack_serializer` | Binary | MessagePack | Compact, fast | High-performance, network transfer |

### `serialization_format` Enum

```cpp
enum class serialization_format {
    binary,      // Custom binary format (@header{};@data{};)
    json,        // JSON format
    xml,         // XML format
    msgpack,     // MessagePack binary format
    auto_detect, // Auto-detect format during deserialization
    unknown      // Unknown or unrecognized format
};
```

### `serializer_strategy` — Base Interface

| Method | Return Type | Description |
|--------|-------------|-------------|
| `serialize(container)` | `Result<vector<uint8_t>>` | Serialize container to bytes (requires `CONTAINER_HAS_RESULT`) |
| `format()` | `serialization_format` | Get the format this serializer handles |
| `name()` | `string_view` | Human-readable name (e.g., "JSON", "MessagePack") |

> **Note**: The `serialize()` method is only available when `CONTAINER_HAS_RESULT` is
> defined, as it returns `kcenon::common::Result<std::vector<uint8_t>>` from
> common_system.

### `serializer_factory` — Creating Serializers

| Method | Description |
|--------|-------------|
| `create(format)` | Create serializer instance; returns `nullptr` for `auto_detect`/`unknown` |
| `is_supported(format)` | Check if format is supported for serialization |

### Usage Examples

**Using the factory:**

```cpp
#include <core/serializers/serializer_factory.h>
using namespace container_module;

// Create a JSON serializer
auto serializer = serializer_factory::create(serialization_format::json);
if (serializer) {
    auto result = serializer->serialize(my_container);
#if CONTAINER_HAS_RESULT
    if (result.is_ok()) {
        auto& bytes = result.value();
        // Use serialized bytes
    }
#endif
}
```

**Direct serializer usage:**

```cpp
#include <core/serializers/json_serializer.h>
using namespace container_module;

json_serializer json;
assert(json.format() == serialization_format::json);
assert(json.name() == "JSON");

#if CONTAINER_HAS_RESULT
// Serialize to bytes
auto result = json.serialize(container);

// JSON and Binary serializers also support string output
std::string json_string = json.serialize_to_string(container);
#endif
```

**Iterating all formats:**

```cpp
for (auto fmt : {serialization_format::json,
                 serialization_format::binary,
                 serialization_format::xml,
                 serialization_format::msgpack}) {
    if (serializer_factory::is_supported(fmt)) {
        auto ser = serializer_factory::create(fmt);
        // Use ser->name() and ser->serialize()
    }
}
```

### MessagePack Serializer

MessagePack is the most compact binary format available. The `msgpack_serializer`
provides the same `serializer_strategy` interface as other serializers.

**Type Mapping (container_system → MessagePack):**

| container_module Type | MessagePack Type |
|-----------------------|-----------------|
| `null_value` | nil |
| `bool_value` | bool |
| `short_value` / `int_value` / `long_value` | int |
| `ushort_value` / `uint_value` / `ulong_value` | uint |
| `float_value` | float32 |
| `double_value` | float64 |
| `string_value` | str |
| `bytes_value` | bin |
| `container_value` | map |
| `array_value` | array |

```cpp
#include <core/serializers/msgpack_serializer.h>
using namespace container_module;

msgpack_serializer msgpack;
assert(msgpack.format() == serialization_format::msgpack);
assert(msgpack.name() == "MessagePack");

#if CONTAINER_HAS_RESULT
auto result = msgpack.serialize(container);
if (result.is_ok()) {
    // result.value() is the most compact serialized form
}
#endif
```

### Additional String Serialization

JSON, Binary, and XML serializers provide an additional `serialize_to_string()`
method not available in the base interface:

| Serializer | Extra Method | Output |
|-----------|-------------|--------|
| `json_serializer` | `serialize_to_string(container)` | JSON string |
| `binary_serializer` | `serialize_to_string(container)` | Binary string representation |
| `xml_serializer` | `serialize_to_string(container)` | XML string |
| `msgpack_serializer` | — | *(not available — binary-only format)* |

---

## 2. Schema Validation System

### Architecture

`container_schema` provides runtime validation of `value_container` contents
against predefined rules. It uses a fluent (builder) API for schema definition.

```
container_schema
├── Field Definitions
│   ├── require(key, type)              → Required field with type
│   ├── require(key, type, nested)      → Required field with nested schema
│   ├── optional(key, type)             → Optional field with type
│   └── optional(key, type, nested)     → Optional field with nested schema
├── Constraints
│   ├── range<integral>(key, min, max)  → Integer range
│   ├── range<floating>(key, min, max)  → Float/double range
│   ├── length(key, min, max)           → String/bytes length
│   ├── pattern(key, regex)             → Regex pattern match
│   ├── one_of(key, allowed)            → Enum-style allowed values
│   └── custom(key, validator_fn)       → Custom validator function
└── Validation
    ├── validate(container)             → First error (optional<validation_error>)
    ├── validate_all(container)         → All errors (vector<validation_error>)
    └── validate_result(container)      → VoidResult (requires CONTAINER_HAS_RESULT)
```

### Supported Value Types

Schemas reference the `value_types` enum for type checking:

| Enum Value | C++ Storage Type | Description |
|------------|------------------|-------------|
| `null_value` | `std::monostate` | Null / empty |
| `bool_value` | `bool` | Boolean |
| `short_value` | `int16_t` | 16-bit signed integer |
| `ushort_value` | `uint16_t` | 16-bit unsigned integer |
| `int_value` | `int32_t` | 32-bit signed integer |
| `uint_value` | `uint32_t` | 32-bit unsigned integer |
| `long_value` | `int32_t` | 32-bit signed (platform alias) |
| `ulong_value` | `uint32_t` | 32-bit unsigned (platform alias) |
| `llong_value` | `int64_t` | 64-bit signed integer |
| `ullong_value` | `uint64_t` | 64-bit unsigned integer |
| `float_value` | `float` | 32-bit floating point |
| `double_value` | `double` | 64-bit floating point |
| `string_value` | `std::string` | UTF-8 string |
| `bytes_value` | `std::vector<uint8_t>` | Binary data |
| `container_value` | `shared_ptr<value_container>` | Nested container |
| `array_value` | *(reserved — not yet in `value_variant`)* | Array of values (planned) |

### Validation Error Codes

| Code | Constant | Description |
|------|----------|-------------|
| 310 | `validation_codes::missing_required` | Required field is missing |
| 311 | `validation_codes::type_mismatch` | Field type does not match schema |
| 312 | `validation_codes::out_of_range` | Numeric value outside range |
| 313 | `validation_codes::invalid_length` | String/bytes length outside bounds |
| 314 | `validation_codes::pattern_mismatch` | String does not match regex |
| 315 | `validation_codes::not_in_allowed_values` | Value not in allowed list |
| 316 | `validation_codes::custom_validation_failed` | Custom validator returned failure |
| 317 | `validation_codes::nested_validation_failed` | Nested container validation failed |

### `validation_error` Structure

```cpp
struct validation_error {
    std::string field;    // Field name that failed
    std::string message;  // Human-readable error message
    int code;             // Error code from validation_codes

    // Factory methods:
    static validation_error missing_required(string_view field_name);
    static validation_error type_mismatch(string_view field, value_types expected, value_types actual);
    static validation_error out_of_range(string_view field, T value, T min, T max);
    static validation_error invalid_length(string_view field, size_t actual, size_t min, size_t max);
    static validation_error pattern_mismatch(string_view field, string_view pattern);
    static validation_error not_allowed(string_view field, string_view value);
    static validation_error custom_failed(string_view field, string_view reason);
    static validation_error nested_failed(string_view field, const vector<validation_error>& errors);
};
```

### Custom Validators

```cpp
using validator_fn = std::function<std::optional<std::string>(const optimized_value&)>;
```

Return `std::nullopt` on success, or an error message string on failure.

### Usage Examples

**Basic schema with type and range constraints:**

```cpp
#include <core/container/schema.h>
using namespace container_module;

auto user_schema = container_schema()
    .require("user_id", value_types::string_value)
    .require("age", value_types::int_value)
    .range("age", 0, 150)
    .optional("phone", value_types::string_value);

// Validate — stops at first error
auto error = user_schema.validate(*container);
if (error) {
    std::cerr << "Field: " << error->field
              << " Error: " << error->message << "\n";
}

// Validate all — collects every error
auto errors = user_schema.validate_all(*container);
for (const auto& e : errors) {
    std::cerr << "[" << e.code << "] " << e.field << ": " << e.message << "\n";
}
```

**String constraints (length, pattern, enum):**

```cpp
auto config_schema = container_schema()
    .require("email", value_types::string_value)
    .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)")
    .length("email", 5, 254)
    .require("role", value_types::string_value)
    .one_of("role", {"admin", "user", "guest"})
    .require("password", value_types::string_value)
    .length("password", 8, 128);
```

**Nested schema validation:**

```cpp
auto address_schema = container_schema()
    .require("street", value_types::string_value)
    .require("city", value_types::string_value)
    .require("zip", value_types::string_value)
    .pattern("zip", R"(^\d{5}(-\d{4})?$)");

auto user_schema = container_schema()
    .require("name", value_types::string_value)
    .require("address", value_types::container_value, address_schema);
```

**Custom validator:**

```cpp
auto schema = container_schema()
    .require("score", value_types::double_value)
    .custom("score", [](const optimized_value& val) -> std::optional<std::string> {
        if (auto* d = std::get_if<double>(&val.data)) {
            if (*d != *d) {  // NaN check
                return "Score must not be NaN";
            }
        }
        return std::nullopt;  // Success
    });
```

**Result-based validation (with common_system):**

```cpp
#if SCHEMA_HAS_COMMON_RESULT
auto result = schema.validate_result(*container);
if (!result.is_ok()) {
    auto& error = result.error();
    // error.code(), error.message(), error.source()
}
#endif
```

### Schema Introspection

| Method | Return | Description |
|--------|--------|-------------|
| `field_count()` | `size_t` | Number of fields defined |
| `has_field(key)` | `bool` | Check if field is defined |
| `is_required(key)` | `bool` | Check if field is required |

---

## 3. Policy Container Framework

### Architecture

`basic_value_container<StoragePolicy>` is a template that decouples storage
behavior from the container API through the **StoragePolicy** concept.

```
basic_value_container<StoragePolicy>
│
├── StoragePolicy = dynamic_storage_policy  (default)
│   └── O(n) lookup, preserves insertion order, vector backend
│
├── StoragePolicy = indexed_storage_policy
│   └── O(1) avg lookup, hash-indexed, vector + unordered_map
│
└── StoragePolicy = static_storage_policy<Types...>
    └── Compile-time type restriction, vector backend
```

### The `StoragePolicy` Concept

All storage policies must satisfy this C++20 concept:

```cpp
template<typename P>
concept StoragePolicy = requires(P p, const P cp, std::string_view key) {
    // Type requirements
    typename P::value_type;
    typename P::iterator;
    typename P::const_iterator;

    // CRUD operations
    { p.set(key, std::declval<const typename P::value_type&>()) } -> std::same_as<void>;
    { cp.get(key) } -> std::same_as<std::optional<typename P::value_type>>;
    { cp.contains(key) } -> std::convertible_to<bool>;
    { p.remove(key) } -> std::convertible_to<bool>;
    { p.clear() } -> std::same_as<void>;

    // Iteration support
    { p.begin() } -> std::same_as<typename P::iterator>;
    { p.end() } -> std::same_as<typename P::iterator>;
    { cp.begin() } -> std::same_as<typename P::const_iterator>;
    { cp.end() } -> std::same_as<typename P::const_iterator>;
    { cp.size() } -> std::convertible_to<std::size_t>;
    { cp.empty() } -> std::convertible_to<bool>;
};
```

### Storage Policies Comparison

| Policy | Lookup | Insert | Remove | Memory | Order | Best For |
|--------|--------|--------|--------|--------|-------|----------|
| `dynamic_storage_policy` | O(n) | O(1)* | O(n) | Low | Insertion | Small containers (<100 keys) |
| `indexed_storage_policy` | O(1) avg | O(1) avg | O(n)** | Medium | Insertion | Large containers (100+ keys) |
| `static_storage_policy<Ts...>` | O(n) | O(1)* | O(n) | Low | Insertion | Type-restricted scenarios |

\* Amortized
\*\* Requires index rebuild

### Type Aliases

```cpp
// Default container (dynamic storage, O(n) lookup)
using policy_container = basic_value_container<policy::dynamic_storage_policy>;

// Fast lookup container (hash-indexed, O(1) average lookup)
using fast_policy_container = basic_value_container<policy::indexed_storage_policy>;

// Type-restricted container (compile-time type safety)
template<typename... AllowedTypes>
using typed_policy_container = basic_value_container<policy::static_storage_policy<AllowedTypes...>>;
```

### Thread Safety Model

`basic_value_container` uses `std::shared_mutex` for thread safety:

| Operation | Lock Type | Concurrent Access |
|-----------|-----------|-------------------|
| `get()`, `contains()`, `size()`, `empty()` | `shared_lock` | Multiple concurrent reads allowed |
| `set()`, `remove()`, `clear_value()` | `unique_lock` | Exclusive access |
| `begin()`/`end()` iterators | No lock | **Not thread-safe** — use external synchronization |

### `basic_value_container<StoragePolicy>` API

#### Value Management

| Method | Description |
|--------|-------------|
| `set<T>(key, value)` | Set typed value; returns `*this` for chaining |
| `set(optimized_value)` | Set pre-built value; returns `*this` for chaining |
| `set_all(span<const optimized_value>)` | Set multiple values at once |
| `get(key)` | Get value by key; returns `optional<optimized_value>` |
| `contains(key)` | Check if key exists |
| `remove(key)` | Remove by key; returns `true` if removed |
| `clear_value()` | Clear all values |

#### Batch Operations

| Method | Description |
|--------|-------------|
| `bulk_insert(vector<optimized_value>&&)` | Move-insert multiple values |
| `bulk_insert(span, reserve_hint)` | Copy-insert with optional pre-allocation |
| `get_batch(span<string_view>)` | Get multiple values; returns `vector<optional>` |
| `contains_batch(span<string_view>)` | Check multiple keys; returns `vector<bool>` |
| `remove_batch(span<string_view>)` | Remove multiple keys; returns count removed |

#### Header Management

| Method | Description |
|--------|-------------|
| `set_source(id, sub_id)` | Set source identifiers |
| `set_target(id, sub_id)` | Set target identifiers |
| `set_message_type(type)` | Set message type |
| `swap_header()` | Swap source and target IDs |
| `source_id()` / `target_id()` / `message_type()` / `version()` | Accessors |

#### Result-based API (with `CONTAINER_HAS_RESULT`)

| Method | Description |
|--------|-------------|
| `get<T>(key)` | Get typed value; returns `Result<T>` |
| `set_result<T>(key, value)` | Set value; returns `VoidResult` |

### Usage Examples

**Default policy container:**

```cpp
#include <core/policy_container.h>
using namespace container_module;

auto container = std::make_shared<policy_container>();
container->set("name", std::string("test"))
         .set("count", static_cast<int64_t>(42))
         .set("rate", 3.14);

auto name = container->get("name");  // optional<optimized_value>
if (name) {
    auto* str = std::get_if<std::string>(&name->data);
    // *str == "test"
}
```

**Fast lookup container:**

```cpp
auto fast = std::make_shared<fast_policy_container>();

// Pre-allocate for known size
fast->reserve(1000);

// Bulk insert
std::vector<optimized_value> values;
// ... populate values ...
fast->bulk_insert(std::move(values));

// O(1) average lookup
auto result = fast->get("key_500");
```

**Type-restricted container:**

```cpp
// Only allow int64_t, double, and string
typed_policy_container<int64_t, double, std::string> typed;

typed.set("count", static_cast<int64_t>(42));  // OK
typed.set("rate", 3.14);                        // OK
typed.set("name", std::string("test"));          // OK
typed.set("flag", true);                         // Silently ignored (bool not allowed)
```

**Compile-time type checking with `static_storage_policy` directly:**

```cpp
using namespace container_module::policy;

static_storage_policy<int64_t, double> storage;
storage.set_typed("count", static_cast<int64_t>(42));  // OK — compile-time verified
// storage.set_typed("flag", true);  // Compile error: bool not in AllowedTypes

auto val = storage.get_typed<int64_t>("count");  // optional<int64_t>
```

**Custom storage policy:**

```cpp
struct my_storage_policy {
    using value_type = optimized_value;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;

    void set(std::string_view key, const value_type& value);
    std::optional<value_type> get(std::string_view key) const;
    bool contains(std::string_view key) const;
    bool remove(std::string_view key);
    void clear();
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    std::size_t size() const;
    bool empty() const;
    void reserve(std::size_t);
    std::vector<value_type>& data();
    const std::vector<value_type>& data() const;
};

// Verify at compile time:
static_assert(container_module::policy::StoragePolicy<my_storage_policy>);

// Use with basic_value_container:
using my_container = container_module::basic_value_container<my_storage_policy>;
```

---

## 4. Integration Examples

### Example 1: Serialize, Validate, Store Pipeline

```cpp
#include <core/policy_container.h>
#include <core/container/schema.h>
#include <core/serializers/serializer_factory.h>
using namespace container_module;

// Define schema
auto user_schema = container_schema()
    .require("user_id", value_types::string_value)
    .require("name", value_types::string_value)
    .length("name", 1, 100)
    .require("age", value_types::int_value)
    .range("age", 0, 150)
    .optional("email", value_types::string_value)
    .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

// Create and populate container
auto container = std::make_shared<policy_container>();
container->set("user_id", std::string("usr_001"))
         .set("name", std::string("Alice"))
         .set("age", static_cast<int32_t>(30))
         .set("email", std::string("alice@example.com"));

// Validate before serialization
auto errors = user_schema.validate_all(*container);
if (!errors.empty()) {
    for (const auto& e : errors) {
        std::cerr << e.field << ": " << e.message << "\n";
    }
    return;
}

// Serialize to MessagePack (most compact)
auto serializer = serializer_factory::create(serialization_format::msgpack);
#if CONTAINER_HAS_RESULT
auto result = serializer->serialize(*container);
if (result.is_ok()) {
    auto& bytes = result.value();
    // Store or transmit bytes
}
#endif
```

### Example 2: Multi-Format Serialization Comparison

```cpp
#include <core/serializers/serializer_factory.h>
using namespace container_module;

void compare_formats(const value_container& container) {
    for (auto fmt : {serialization_format::json,
                     serialization_format::binary,
                     serialization_format::xml,
                     serialization_format::msgpack}) {
        auto ser = serializer_factory::create(fmt);
        if (!ser) continue;

#if CONTAINER_HAS_RESULT
        auto result = ser->serialize(container);
        if (result.is_ok()) {
            std::cout << ser->name() << ": "
                      << result.value().size() << " bytes\n";
        }
#endif
    }
}
// Expected output (approximate):
// JSON: 150 bytes
// Binary: 120 bytes
// XML: 200 bytes
// MessagePack: 80 bytes
```

### Example 3: Fast Lookup with Schema Validation

```cpp
#include <core/policy_container.h>
#include <core/container/schema.h>
using namespace container_module;

// Schema for configuration data
auto config_schema = container_schema()
    .require("max_connections", value_types::int_value)
    .range("max_connections", 1, 10000)
    .require("timeout_ms", value_types::int_value)
    .range("timeout_ms", 100, 60000)
    .require("host", value_types::string_value)
    .length("host", 1, 253)
    .optional("port", value_types::int_value)
    .range("port", 1, 65535);

// Use fast_policy_container for O(1) lookups
auto config = std::make_shared<fast_policy_container>();
config->set("max_connections", static_cast<int32_t>(100));
config->set("timeout_ms", static_cast<int32_t>(5000));
config->set("host", std::string("api.example.com"));
config->set("port", static_cast<int32_t>(8443));

// Validate configuration
auto error = config_schema.validate(*config);
if (error) {
    std::cerr << "Config invalid: " << error->message << "\n";
} else {
    // O(1) lookups in hot path
    auto timeout = config->get("timeout_ms");
}
```

### Example 4: Type-Safe Protocol Container

```cpp
#include <core/policy_container.h>
using namespace container_module;

// Only allow specific types for protocol messages
typed_policy_container<int64_t, std::string, std::vector<uint8_t>> message;

// Set header
message.set_source("client_01", "session_42");
message.set_target("server_01", "");
message.set_message_type("data_request");

// Set payload (only int64_t, string, and bytes allowed)
message.set("request_id", static_cast<int64_t>(12345));
message.set("query", std::string("SELECT * FROM users"));
message.set("auth_token", std::vector<uint8_t>{0xDE, 0xAD, 0xBE, 0xEF});

// This would be silently ignored (bool not in AllowedTypes):
// message.set("verbose", true);
```

---

## 5. Conditional Compilation

Several features depend on common_system integration:

| Macro | Defined When | Enables |
|-------|-------------|---------|
| `CONTAINER_HAS_RESULT` | common_system linked | `Result<T>` return types in serializers |
| `SCHEMA_HAS_COMMON_RESULT` | same as above (alias) | `validate_result()` in schema |

When `CONTAINER_HAS_RESULT` is not defined:
- `serializer_strategy::serialize()` is not available
- `validate_result()` is not available
- `get<T>()` / `set_result<T>()` Result-based API is not available
- All other APIs work normally

---

## References

- **Source files**:
  - `core/serializers/serializer_strategy.h` — Base interface (122 lines)
  - `core/serializers/serializer_factory.h` — Factory class (91 lines)
  - `core/serializers/json_serializer.h` — JSON format (102 lines)
  - `core/serializers/binary_serializer.h` — Binary format (102 lines)
  - `core/serializers/xml_serializer.h` — XML format (102 lines)
  - `core/serializers/msgpack_serializer.h` — MessagePack format (92 lines)
  - `core/container/schema.h` — Schema validation (624 lines)
  - `core/policy_container.h` — Policy container template (636 lines)
  - `core/storage_policy.h` — Storage policies and concept (684 lines)
- **Tests**:
  - `tests/msgpack_tests.cpp` — MessagePack encoder/decoder tests
  - `tests/schema_tests.cpp` — Schema validation tests
  - `tests/policy_container_tests.cpp` — Policy container tests
  - `tests/storage_policy_tests.cpp` — Storage policy tests
- **Related docs**:
  - [Async Guide](ASYNC_GUIDE.md) — C++20 coroutine async framework
  - [Lock-Free Data Structures](advanced/LOCK_FREE.md) — RCU and epoch-based reclamation
  - [Memory Management](advanced/MEMORY_MANAGEMENT.md) — Memory pool architecture
