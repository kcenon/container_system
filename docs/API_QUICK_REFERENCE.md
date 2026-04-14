# API Quick Reference -- container_system

Cheat-sheet for the most common `container_system` APIs.
For full details see [API_REFERENCE.md](API_REFERENCE.md) and the Doxygen-generated docs.

---

## Header and Namespace

```cpp
#include <kcenon/container/container.h>

using namespace kcenon::container;
```

`message_buffer` is the preferred alias for `value_container` (introduced in the v0.1 baseline; see `CLAUDE.md` Architecture notes).

---

## Container Creation

```cpp
// Default container (type "data_container", version "1.0.0.0")
auto c = std::make_shared<value_container>();

// From serialized string (lazy header-only parse by default)
auto c = std::make_shared<value_container>(json_string);

// From raw bytes
auto c = std::make_shared<value_container>(byte_vector);

// Using the preferred alias
message_buffer msg;
```

---

## Value Types (16 types)

| Enum constant | Index | C++ type |
|---|---|---|
| `null_value` | 0 | (none) |
| `bool_value` | 1 | `bool` |
| `short_value` | 2 | `int16_t` |
| `ushort_value` | 3 | `uint16_t` |
| `int_value` | 4 | `int32_t` |
| `uint_value` | 5 | `uint32_t` |
| `long_value` | 6 | `int32_t` (platform) |
| `ulong_value` | 7 | `uint32_t` (platform) |
| `llong_value` | 8 | `int64_t` |
| `ullong_value` | 9 | `uint64_t` |
| `float_value` | 10 | `float` |
| `double_value` | 11 | `double` |
| `string_value` | 12 | `std::string` |
| `bytes_value` | 13 | `std::vector<uint8_t>` |
| `container_value` | 14 | nested `value_container` |
| `array_value` | 15 | array of values |

Compile-time helpers:

```cpp
constexpr auto t = get_type_from_string("4");   // value_types::int_value
constexpr auto s = get_string_from_type(value_types::int_value); // "4"
```

---

## Value Operations

### Set (unified API -- method chaining)

```cpp
container->set("name", "Alice")
         .set("age", 30)
         .set("score", 99.5);

// Set from optimized_value
container->set(optimized_value{"key", value_variant{42}, value_types::int_value});

// Set multiple at once
container->set_all(values_span);
```

### Get (unified API)

```cpp
// Single value
auto val = container->get("name");          // std::optional<optimized_value>

// Zero-copy view
auto view = container->get("name", as_view); // std::optional<value_view>

// Batch get (vector)
auto vals = container->get(keys_span);

// Batch get (map)
auto map = container->get_as_map(keys_span);

// Typed get with Result
auto result = container->get<std::string>("name"); // Result<std::string>
```

### Contains / Remove

```cpp
bool exists = container->contains("name");
container->remove_result("name");          // VoidResult

// Batch
auto flags = container->contains_batch(keys_span);
size_t n   = container->remove_batch(keys_span);
```

### Bulk Insert

```cpp
container->bulk_insert(std::move(values_vec));
container->bulk_insert(values_span, /*reserve_hint=*/100);
```

### Conditional Update (CAS)

```cpp
bool ok = container->update_if("counter", /*expected=*/old_val, /*new=*/new_val);
auto results = container->update_batch_if(update_specs);
```

---

## Serialization

### Formats

| `serialization_format` | Description |
|---|---|
| `binary` | Custom binary (`@header{};@data{};`) |
| `json` | JSON text |
| `xml` | XML text |
| `msgpack` | MessagePack binary |
| `auto_detect` | Auto-detect on deserialization |

### Serialize

```cpp
// To bytes
auto bytes = container->serialize(serialization_format::json);

// To string (convenient for JSON/XML)
auto str = container->serialize_string(serialization_format::json);
if (str.is_ok()) {
    std::cout << str.value();
}
```

### Deserialize

```cpp
// Auto-detect format
auto result = container->deserialize(data_span);

// Explicit format
auto result = container->deserialize(data_span, serialization_format::msgpack);

// From string (legacy)
auto c = std::make_shared<value_container>(json_string);
```

### Serializer Factory (strategy pattern)

```cpp
auto serializer = serializer_factory::create(serialization_format::json);
bool supported  = serializer_factory::is_supported(serialization_format::xml);
```

### File I/O

```cpp
auto r1 = container->load_packet_result("/path/data.bin");
auto r2 = container->save_packet_result("/path/data.bin");
```

---

## Schema Validation

```cpp
#include <kcenon/container/container.h>  // includes schema.h

auto schema = container_schema()
    .require("user_id", value_types::string_value)
    .require("age", value_types::int_value)
    .range("age", 0, 150)
    .optional("phone", value_types::string_value);

auto result = schema.validate(*container);
if (!result) {
    for (const auto& err : container->get_validation_errors()) {
        std::cerr << err.field << ": " << err.message << "\n";
    }
}

// Schema-validated deserialization
auto r = container->deserialize_result(data, schema);
```

---

## Async Coroutine Patterns (C++20)

> Requires `CONTAINER_ENABLE_COROUTINES=ON` (default).

```cpp
#include <kcenon/container/internal/async/task.h>
#include <kcenon/container/internal/async/generator.h>
#include <kcenon/container/internal/async/async_container.h>

// task<T> -- single async result
task<value_container> fetch_data() {
    auto container = std::make_shared<value_container>();
    // ... populate asynchronously ...
    co_return *container;
}

// generator<T> -- lazy sequence
generator<optimized_value> stream_values(const value_container& c) {
    for (auto& v : c.get_variant_values()) {
        co_yield v;
    }
}
```

---

## Message Buffer (messaging integration)

```cpp
// message_buffer is the preferred alias for value_container
message_buffer msg;

// Header fields
msg.set_source("client-1", "sub-1");
msg.set_target("server-1");
msg.set_message_type("request");

// Swap source/target
msg.swap_header();

// Read header
auto src = msg.source_id();
auto tgt = msg.target_id();
auto typ = msg.message_type();

// Copy (with or without values)
auto header_only = msg.copy(/*containing_values=*/false);
auto full_copy   = msg.copy(/*containing_values=*/true);
```

---

## Thread Safety

All `value_container` operations are thread-safe by default (since v0.2.0).
For advanced lock-free reads, use `thread_safe_container` with RCU semantics.

```cpp
#include <kcenon/container/internal/thread_safe_container.h>

// thread_safe_container wraps value_container
// - Write operations use mutex
// - Read operations use RCU (rcu_value + epoch_manager) for lock-free access
```

---

## Quick Recipes

**Round-trip JSON serialization**

```cpp
auto c = std::make_shared<value_container>();
c->set("greeting", "hello").set("count", 42);

auto json = c->serialize_string(serialization_format::json);

auto c2 = std::make_shared<value_container>();
c2->deserialize(std::vector<uint8_t>(json.value().begin(), json.value().end()));
```

**Schema-validated message processing**

```cpp
auto schema = container_schema()
    .require("action", value_types::string_value)
    .require("payload", value_types::container_value);

auto msg = std::make_shared<value_container>(incoming_data);
auto result = schema.validate(*msg);
if (result) {
    auto action = msg->get<std::string>("action");
    // process ...
}
```
