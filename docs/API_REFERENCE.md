# container_system API Reference

> **Version**: 2.0
> **Last Updated**: 2025-11-21
> **Status**: Migrating to variant_value_v2 (Phase 2 in progress)

## Table of Contents

1. [Namespace](#namespace)
2. [variant_value_v2 (Recommended)](#variant_value_v2-recommended)
3. [Container](#container)
4. [Serialization/Deserialization](#serializationdeserialization)

---

## Namespace

### `kcenon::container`

All public APIs of container_system are contained in this namespace

**Included Items**:
- `variant_value_v2` - Next-generation Value system (recommended)
- `container` - Value container
- Serialization functions

---

## variant_value_v2 (Recommended)

### Overview

**Header**: `#include <kcenon/container/values/variant_value_v2.h>`

**Description**: High-performance type-safe variant value implementation (4.39x performance improvement)

**Supported Types**:
- Primitive types: `bool`, `int`, `uint`, `long`, `float`, `double`, `string`
- Container types: `vector`, `map`
- Binary types: `bytes`

### Creation and Access

```cpp
#include <kcenon/container/values/variant_value_v2.h>

using namespace kcenon::container;

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

**Header**: `#include <kcenon/container/container.h>`

**Description**: Key-Value store

#### Creation and Usage

```cpp
#include <kcenon/container/container.h>

using namespace kcenon::container;

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

## Serialization/Deserialization

### JSON Serialization

```cpp
#include <kcenon/container/serialization.h>

using namespace kcenon::container;

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
- **Phase 2**: 🔄 In Progress
  - Core container migration
  - Factory function implementation
- **Phase 3-5**: ⏳ Pending

### Recommendations

- **New code**: Use variant_value_v2 (recommended)
- **Existing code**: Gradual migration (see MIGRATION_GUIDE)

---

**Created**: 2025-11-21
**Version**: 2.0
**Author**: container_system team
