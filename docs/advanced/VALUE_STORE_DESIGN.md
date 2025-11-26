# value_store Design Document

**Date**: 2025-11-09
**Status**: Phase 1 Completed (Interface Definition)
**Sprint**: Sprint 3 - Domain Separation

---

## Overview

`value_store` is a domain-agnostic, general-purpose value storage layer extracted from `value_container` as part of the Domain Separation initiative (Sprint 3).

### Purpose

- **Separation of Concerns**: Pure value storage without messaging-specific fields
- **Reusability**: Can be used as a standalone serialization library
- **Type Safety**: Built on top of `variant_value_v2` for compile-time type checking
- **Performance**: Includes Small Object Optimization (SOO) for better performance

---

## Architecture

```
┌───────────────────────────────────────┐
│      message_container (Sprint 3)    │
│  (Messaging-specific wrapper)         │
│  - source_id, target_id, message_type│
└───────────────┬───────────────────────┘
                │ Composition
                ▼
┌───────────────────────────────────────┐
│         value_store (Phase 1)         │
│  (Domain-agnostic storage)            │
│  - Key-value storage                  │
│  - Serialization/Deserialization      │
│  - Thread-safe operations             │
└───────────────┬───────────────────────┘
                │ Uses
                ▼
┌───────────────────────────────────────┐
│       variant_value_v2 (Core)         │
│  (Type-safe variant storage)          │
│  - Correct type ordering              │
│  - std::variant based                 │
│  - std::visit support                 │
└───────────────────────────────────────┘
```

---

## Implementation Phases

### Phase 1: Interface Definition ✅ (2025-11-09)

**Completed:**
- Created `value_store.h` with full API specification
- Implemented `value_store.cpp` with basic operations
- Added to CMakeLists.txt build system
- Successfully compiled with container_system

**API Surface:**
```cpp
class value_store {
    // Value Management
    void add(const std::string& key, variant_value_v2 val);
    std::optional<variant_value_v2> get(const std::string& key) const;
    bool contains(const std::string& key) const;
    bool remove(const std::string& key);
    void clear();
    size_t size() const;
    bool empty() const;

    // Serialization (TODO: Phase 2)
    std::string serialize() const;
    std::vector<uint8_t> serialize_binary() const;
    static value_store deserialize(std::string_view json_data);
    static value_store deserialize_binary(const std::vector<uint8_t>&);

    // Thread Safety
    void enable_thread_safety();
    void disable_thread_safety();
    bool is_thread_safe() const;

    // Statistics
    size_t get_read_count() const;
    size_t get_write_count() const;
    void reset_statistics();
};
```

### Phase 2: Serialization Implementation ✅ (2025-11-26)

**Completed:**
- Implemented JSON serialization (`serialize()`) - outputs human-readable JSON
- Implemented binary serialization (`serialize_binary()`) with version byte for forward compatibility
- Implemented binary deserialization (`deserialize_binary()`) with round-trip support
- JSON deserialization deferred (requires JSON parser library)
- Added comprehensive unit tests (11 tests covering various scenarios)
- Thread-safe serialization support

**API Changes:**
- `deserialize()` and `deserialize_binary()` now return `std::unique_ptr<value_store>` instead of `value_store` (due to deleted move constructor)

**Binary Format (v1):**
```
[version:1][count:4]([key_len:4][key:UTF-8][value_len:4][value_serialized])...
```

### Phase 3: message_container Wrapper (TODO - Sprint 3 continuation)

**Planned Tasks:**
1. Create `message_container` class wrapping `value_store`
2. Add messaging-specific fields (source_id, target_id, etc.)
3. Migrate existing `value_container` users to `message_container`
4. Add deprecation notice to `value_container`

---

## Design Decisions

### 1. Why Key-Value Storage?

**Decision**: Use `std::unordered_map<std::string, variant_value_v2>` for storage

**Rationale:**
- Simple and efficient O(1) average lookup
- Flexible key-based access pattern
- Easy to serialize to JSON/Binary formats
- Matches existing container_system usage patterns

**Alternatives Considered:**
- Vector-based storage: Less flexible, harder to look up by name
- B-tree based storage: Overkill for typical use cases

### 2. Thread Safety Model

**Decision**: Optional thread-safety via `enable_thread_safety()` flag

**Rationale:**
- Zero overhead when not needed (single-threaded use cases)
- Explicit opt-in prevents accidental performance loss
- Uses `std::shared_mutex` for reader-writer optimization

**Pattern:**
```cpp
value_store store;
store.enable_thread_safety();  // Explicit opt-in

// All subsequent operations are thread-safe
store.add("key", value);
auto val = store.get("key");
```

### 3. Serialization Deferred to Phase 2

**Decision**: Stub out serialization methods in Phase 1

**Rationale:**
- Phase 1 focuses on interface stability
- Serialization requires careful design (format versioning, compatibility)
- Allows parallel work on other systems (logger, thread, etc.)
- Can leverage existing container serialization code in Phase 2

---

## Usage Examples

### Basic Usage

```cpp
#include <container/core/value_store.h>
#include <container/internal/variant_value_v2.h>

using namespace container_module;

value_store store;

// Add values
store.add("name", variant_value_v2("John Doe"));
store.add("age", variant_value_v2(30));
store.add("active", variant_value_v2(true));

// Retrieve values
auto name = store.get("name");
if (name) {
    // Use std::visit to access value
    std::visit([](auto&& val) {
        std::cout << val << std::endl;
    }, name->get_variant());
}

// Check existence
if (store.contains("age")) {
    std::cout << "Age exists" << std::endl;
}

// Remove value
store.remove("active");

// Statistics
std::cout << "Read count: " << store.get_read_count() << std::endl;
std::cout << "Write count: " << store.get_write_count() << std::endl;
```

### Thread-Safe Usage

```cpp
value_store shared_store;
shared_store.enable_thread_safety();

// Producer thread
std::thread producer([&] {
    for (int i = 0; i < 1000; ++i) {
        shared_store.add("key_" + std::to_string(i), variant_value_v2(i));
    }
});

// Consumer thread
std::thread consumer([&] {
    for (int i = 0; i < 1000; ++i) {
        auto val = shared_store.get("key_" + std::to_string(i));
        // Process val...
    }
});

producer.join();
consumer.join();
```

---

## Next Steps (Phase 3)

**Phase 2 Completed (2025-11-26):**
- ✅ Binary serialization with version support
- ✅ JSON serialization (output only)
- ✅ Comprehensive unit tests (11 tests)

**Remaining Tasks:**

1. **JSON Deserialization** (Optional - requires JSON parser)
   - Consider adding nlohmann/json or similar library
   - For now, use binary serialization for round-trip

2. **message_container Wrapper** (Phase 3)
   - Create wrapper with messaging fields
   - Migration guide for existing users
   - Deprecation notices

3. **Documentation**
   - API reference
   - Migration guide
   - Performance characteristics

---

## Success Metrics

- ✅ Interface compiles without errors
- ✅ Basic operations (add/get/remove) work correctly
- ✅ Thread-safety opt-in model implemented
- ✅ Serialization round-trip tests pass (Phase 2 - 2025-11-26)
- ✅ Binary serialization with version support for forward compatibility
- ⏳ Zero performance regression vs. current container (Phase 2)
- ⏳ All existing tests pass after migration (Phase 3)

---

## References

- [IMPROVEMENT_PLAN.md](../IMPROVEMENT_PLAN.md) - Sprint 3 roadmap
- [ADR-001-Type-System-Unification.md](../docs/ADR-001-Type-System-Unification.md) - Type system decision
- [VARIANT_VALUE_V2_MIGRATION_GUIDE.md](../docs/VARIANT_VALUE_V2_MIGRATION_GUIDE.md) - Migration guide
