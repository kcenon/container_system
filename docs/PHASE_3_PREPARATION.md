# Phase 3: Error Handling Preparation - container_system

**Version**: 1.0
**Date**: 2025-10-09
**Status**: Ready for Implementation

---

## Overview

This document outlines the migration path for container_system to adopt the centralized error handling from common_system Phase 3.

---

## Current State

### Error Handling Status

**Current Approach**:
- Mix of exceptions and boolean returns
- `deserialize_result()` already uses Result<T> (partial integration)
- Type conversion methods throw `std::logic_error`
- Memory pool operations throw `std::bad_alloc` and `std::runtime_error`

**Example**:
```cpp
// Current: throws exceptions
virtual int to_int() const {
    if (type_ == value_types::null_value)
        throw std::logic_error("Cannot convert null_value to int.");
    // ...
}

// Current: boolean returns
bool deserialize(const std::string& data_string,
                 bool parse_only_header = true);

// Partial: Result<T> already exists
VoidResult deserialize_result(const std::string& data_string,
                               bool parse_only_header = true);
```

---

## Migration Plan

### Phase 3.1: Import Error Codes

**Action**: Add common_system error code dependency

```cpp
#include <kcenon/common/error/error_codes.h>
#include <kcenon/common/patterns/result.h>

using namespace common;
using namespace common::error;
```

### Phase 3.2: Key API Migrations

#### Priority 1: Serialization Operations (High Impact)

```cpp
// Before
bool value_container::deserialize(const std::string& data_string,
                                   bool parse_only_header = true);
std::string value_container::serialize() const;

// After
Result<void> value_container::deserialize(const std::string& data_string,
                                           bool parse_only_header = true);
Result<std::string> value_container::serialize() const;
```

**Error Codes**:
- `codes::container_system::deserialization_failed`
- `codes::container_system::serialization_failed`
- `codes::container_system::invalid_format`

**Example Implementation**:
```cpp
Result<void> value_container::deserialize(
    const std::string& data_string,
    bool parse_only_header
) {
    if (data_string.empty()) {
        return error<std::monostate>(
            codes::container_system::invalid_format,
            "Empty data string",
            "value_container"
        );
    }

    try {
        bool success = deserialize_values(data_string, parse_only_header);
        if (!success) {
            return error<std::monostate>(
                codes::container_system::deserialization_failed,
                "Failed to parse container data",
                "value_container",
                data_string.substr(0, 100)  // First 100 chars for context
            );
        }
        return ok();
    } catch (const std::exception& e) {
        return error<std::monostate>(
            codes::container_system::deserialization_failed,
            "Deserialization exception",
            "value_container",
            e.what()
        );
    }
}

Result<std::string> value_container::serialize() const {
    try {
        // Build serialized string
        std::string result = build_serialized_string();
        return ok(std::move(result));
    } catch (const std::exception& e) {
        return error<std::string>(
            codes::container_system::serialization_failed,
            "Serialization failed",
            "value_container",
            e.what()
        );
    }
}
```

#### Priority 2: Value Type Conversions

```cpp
// Before
virtual int to_int() const;
virtual std::string to_string(bool original = true) const;

// After
virtual Result<int> to_int() const;
virtual Result<std::string> to_string(bool original = true) const;
```

**Error Codes**:
- `codes::container_system::value_type_mismatch`
- `codes::container_system::invalid_value_type`
- `codes::container_system::value_conversion_failed`

**Example Implementation**:
```cpp
Result<int> value::to_int() const {
    if (type_ == value_types::null_value) {
        return error<int>(
            codes::container_system::value_type_mismatch,
            "Cannot convert null_value to int",
            "value"
        );
    }

    if (!is_numeric()) {
        return error<int>(
            codes::container_system::value_type_mismatch,
            "Value is not numeric",
            "value",
            std::string("Type: ") + type_to_string(type_)
        );
    }

    try {
        // Perform conversion based on internal type
        int result = convert_to_int_internal();
        return ok(result);
    } catch (const std::exception& e) {
        return error<int>(
            codes::container_system::value_conversion_failed,
            "Integer conversion failed",
            "value",
            e.what()
        );
    }
}
```

#### Priority 3: Memory Pool Operations

```cpp
// Before
void* fixed_block_pool::allocate();
void fixed_block_pool::deallocate(void* p);

// After
Result<void*> fixed_block_pool::allocate();
Result<void> fixed_block_pool::deallocate(void* p);
```

**Error Codes**:
- `codes::container_system::pool_exhausted`
- `codes::container_system::allocation_failed`
- `codes::container_system::invalid_allocation_size`

**Example Implementation**:
```cpp
Result<void*> fixed_block_pool::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!free_list_) {
        try {
            allocate_chunk_unlocked();
        } catch (const std::bad_alloc& e) {
            return error<void*>(
                codes::container_system::pool_exhausted,
                "Memory pool exhausted",
                "fixed_block_pool",
                std::string("Block size: ") + std::to_string(block_size_)
            );
        } catch (const std::exception& e) {
            return error<void*>(
                codes::container_system::allocation_failed,
                "Chunk allocation failed",
                "fixed_block_pool",
                e.what()
            );
        }

        if (!free_list_) {
            return error<void*>(
                codes::container_system::allocation_failed,
                "Failed to create free list",
                "fixed_block_pool"
            );
        }
    }

    void* p = free_list_;
    free_list_ = *reinterpret_cast<void**>(free_list_);
    ++allocated_count_;
    return ok(p);
}

Result<void> fixed_block_pool::deallocate(void* p) {
    if (!p) {
        return error<std::monostate>(
            codes::common::invalid_argument,
            "Null pointer passed to deallocate",
            "fixed_block_pool"
        );
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Validate pointer belongs to pool
    if (!is_valid_pool_pointer(p)) {
        return error<std::monostate>(
            codes::common::invalid_argument,
            "Pointer does not belong to pool",
            "fixed_block_pool"
        );
    }

    *reinterpret_cast<void**>(p) = free_list_;
    free_list_ = p;
    --allocated_count_;
    return ok();
}
```

#### Priority 4: File I/O Operations

```cpp
// Before
void value_container::load_packet(const std::string& file_path);
void value_container::save_packet(const std::string& file_path);

// After
Result<void> value_container::load_packet(const std::string& file_path);
Result<void> value_container::save_packet(const std::string& file_path);
```

**Error Codes**:
- `codes::logger_system::file_open_failed` (reuse from logger_system)
- `codes::logger_system::file_write_failed`
- `codes::common::io_error`

**Example Implementation**:
```cpp
Result<void> value_container::load_packet(const std::string& file_path) {
    if (file_path.empty()) {
        return error<std::monostate>(
            codes::common::invalid_argument,
            "Empty file path",
            "value_container"
        );
    }

    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return error<std::monostate>(
                codes::logger_system::file_open_failed,
                "Failed to open file",
                "value_container",
                file_path
            );
        }

        // Read file content
        std::string content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        // Deserialize
        auto result = deserialize(content);
        if (result.is_err()) {
            return error<std::monostate>(result.error());
        }

        return ok();
    } catch (const std::exception& e) {
        return error<std::monostate>(
            codes::common::io_error,
            "File load failed",
            "value_container",
            std::string(file_path) + ": " + e.what()
        );
    }
}

Result<void> value_container::save_packet(const std::string& file_path) {
    if (file_path.empty()) {
        return error<std::monostate>(
            codes::common::invalid_argument,
            "Empty file path",
            "value_container"
        );
    }

    // Serialize first
    auto serialize_result = serialize();
    if (serialize_result.is_err()) {
        return error<std::monostate>(serialize_result.error());
    }

    try {
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return error<std::monostate>(
                codes::logger_system::file_open_failed,
                "Failed to open file for writing",
                "value_container",
                file_path
            );
        }

        file << serialize_result.value();
        if (!file.good()) {
            return error<std::monostate>(
                codes::logger_system::file_write_failed,
                "Write operation failed",
                "value_container",
                file_path
            );
        }

        return ok();
    } catch (const std::exception& e) {
        return error<std::monostate>(
            codes::common::io_error,
            "File save failed",
            "value_container",
            std::string(file_path) + ": " + e.what()
        );
    }
}
```

---

## Migration Checklist

### Code Changes

- [ ] Add common_system error code includes
- [ ] Migrate `value_container::deserialize()` to Result<void>
- [ ] Migrate `value_container::serialize()` to Result<std::string>
- [ ] Migrate value type conversion methods to Result<T>
- [ ] Migrate `fixed_block_pool::allocate()` to Result<void*>
- [ ] Migrate `fixed_block_pool::deallocate()` to Result<void>
- [ ] Migrate file I/O operations to Result<void>
- [ ] Update `add()` and `remove()` methods to Result<T>
- [ ] Add error context (type info, file path, etc.)

### Test Updates

- [ ] Update unit tests for Result<T> APIs
- [ ] Add error case tests for each error code
- [ ] Test serialization format errors
- [ ] Test type conversion failures
- [ ] Test memory pool exhaustion
- [ ] Test file I/O errors
- [ ] Verify error message quality

### Documentation

- [ ] Update API reference with Result<T> signatures
- [ ] Document error codes for each function
- [ ] Add migration examples
- [ ] Update integration examples
- [ ] Create error handling guide

---

## Example Migrations

### Example 1: Basic Serialization

```cpp
// Usage before
value_container container;
try {
    if (!container.deserialize(data)) {
        std::cerr << "Deserialization failed" << std::endl;
        return;
    }
} catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return;
}

// Usage after
value_container container;
auto result = container.deserialize(data);
if (result.is_err()) {
    const auto& err = result.error();
    std::cerr << "Deserialization failed: " << err.message << std::endl;
    if (err.details) {
        std::cerr << "Details: " << *err.details << std::endl;
    }
    return;
}
```

### Example 2: Monadic Chaining with Type Conversion

```cpp
// Chain operations
auto result = container->get_value("age")
    ->to_int()
    .and_then([](int age) -> Result<std::string> {
        if (age >= 18) {
            return ok(std::string("adult"));
        }
        return ok(std::string("minor"));
    })
    .or_else([](const error_info& err) {
        log_error("Age validation failed: {}", err.message);
        return ok(std::string("unknown"));
    });
```

### Example 3: Memory Pool with Error Recovery

```cpp
// Attempt allocation with fallback
auto allocate_with_fallback = [&pool]() -> Result<void*> {
    return pool.allocate()
        .or_else([](const error_info& err) -> Result<void*> {
            if (err.code == codes::container_system::pool_exhausted) {
                log_warn("Pool exhausted, using malloc");
                void* ptr = std::malloc(pool.block_size());
                if (ptr) {
                    return ok(ptr);
                }
            }
            return error<void*>(err);
        });
};
```

### Example 4: File I/O with Retry

```cpp
// Load with automatic retry
Result<void> load_with_retry(const std::string& path, int max_attempts = 3) {
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        auto result = container.load_packet(path);
        if (result.is_ok()) {
            return result;
        }

        if (attempt < max_attempts) {
            log_warn("Load attempt {} failed: {}",
                     attempt, result.error().message);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            return error<std::monostate>(result.error());
        }
    }
    return ok();
}
```

---

## Error Code Mapping

### Container System Error Codes (-400 to -499)

```cpp
namespace common::error::codes::container_system {
    // Value errors (-400 to -419)
    constexpr int value_type_mismatch = -400;
    constexpr int invalid_value_type = -401;
    constexpr int value_conversion_failed = -402;

    // Serialization errors (-420 to -439)
    constexpr int serialization_failed = -420;
    constexpr int deserialization_failed = -421;
    constexpr int invalid_format = -422;

    // Memory pool errors (-440 to -459)
    constexpr int pool_exhausted = -440;
    constexpr int allocation_failed = -441;
    constexpr int invalid_allocation_size = -442;

    // Container errors (-460 to -479)
    constexpr int key_not_found = -460;
    constexpr int duplicate_key = -461;
    constexpr int container_full = -462;
}
```

### Error Messages

| Code | Message | When to Use |
|------|---------|-------------|
| value_type_mismatch | "Value type mismatch" | Type conversion not supported |
| value_conversion_failed | "Value conversion failed" | Conversion threw exception |
| serialization_failed | "Serialization failed" | Cannot serialize container |
| deserialization_failed | "Deserialization failed" | Cannot parse data |
| invalid_format | "Invalid format" | Malformed data string |
| pool_exhausted | "Memory pool exhausted" | No free blocks available |
| allocation_failed | "Allocation failed" | Memory allocation error |
| key_not_found | "Key not found" | Value lookup failed |
| container_full | "Container full" | Capacity limit reached |

---

## Testing Strategy

### Unit Tests

```cpp
TEST(ContainerPhase3, DeserializeInvalidFormat) {
    value_container container;
    auto result = container.deserialize("invalid{data}");

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(
        codes::container_system::deserialization_failed,
        result.error().code
    );
}

TEST(ContainerPhase3, TypeConversionMismatch) {
    auto value = std::make_shared<string_value>("test", "hello");
    auto result = value->to_int();

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(
        codes::container_system::value_type_mismatch,
        result.error().code
    );
    EXPECT_EQ("value", result.error().module);
}

TEST(ContainerPhase3, MemoryPoolExhaustion) {
    fixed_block_pool pool(64, 2);  // Small pool

    // Exhaust pool
    std::vector<void*> allocations;
    for (int i = 0; i < 2; ++i) {
        auto result = pool.allocate();
        ASSERT_TRUE(result.is_ok());
        allocations.push_back(result.value());
    }

    // Should grow or fail gracefully
    auto result = pool.allocate();
    // Either succeeds (pool grew) or fails with proper error
    if (result.is_err()) {
        EXPECT_EQ(
            codes::container_system::pool_exhausted,
            result.error().code
        );
    }
}
```

### Integration Tests

```cpp
TEST(ContainerPhase3, SerializationRoundTrip) {
    value_container original;
    original.set_message_type("test");
    original.add(string_value("name", "Alice"));
    original.add(numeric_value("age", 30));

    // Serialize
    auto serialize_result = original.serialize();
    ASSERT_TRUE(serialize_result.is_ok());

    // Deserialize
    value_container restored;
    auto deserialize_result = restored.deserialize(serialize_result.value());
    ASSERT_TRUE(deserialize_result.is_ok());

    // Verify
    EXPECT_EQ("test", restored.message_type());
    auto name_result = restored.get_value("name")->to_string();
    ASSERT_TRUE(name_result.is_ok());
    EXPECT_EQ("Alice", name_result.value());
}

TEST(ContainerPhase3, FileIOWithErrors) {
    value_container container;

    // Test invalid path
    auto load_result = container.load_packet("/nonexistent/path/file.dat");
    ASSERT_TRUE(load_result.is_err());
    EXPECT_EQ(
        codes::logger_system::file_open_failed,
        load_result.error().code
    );

    // Test invalid permissions
    auto save_result = container.save_packet("/root/protected.dat");
    ASSERT_TRUE(save_result.is_err());
}
```

---

## Performance Impact

### Expected Overhead

- **Result<T> size**: +24 bytes per return value (variant overhead)
- **Success path**: ~0-1ns (inline optimization)
- **Error path**: ~2-3ns (error_info construction)

### Current Performance (Baseline)

- **Serialization**: ~2μs for small container (10 values)
- **Deserialization**: ~3μs for small container
- **Type conversion**: ~5ns (simple types)
- **Memory allocation**: ~50ns (from pool)

### Expected After Migration

- **Serialization**: ~2.1μs (-5% due to Result wrapping)
- **Deserialization**: ~3.1μs (-3%)
- **Type conversion**: ~5ns (no change with inline)
- **Memory allocation**: ~52ns (-4%)

**Conclusion**: Negligible performance impact (<5%)

---

## Implementation Timeline

### Week 1: Core APIs
- Day 1-2: Migrate serialization/deserialization
- Day 3: Update value type conversions
- Day 4-5: Update tests

### Week 2: Advanced Features
- Day 1-2: Migrate memory pool operations
- Day 3: Migrate file I/O
- Day 4-5: Integration tests

### Week 3: Finalization
- Day 1-2: Migrate remaining operations (add, remove)
- Day 3: Performance validation
- Day 4-5: Documentation and code review

---

## Backwards Compatibility

### Strategy: Dual API (Transitional)

Provide both old and new APIs during transition:

```cpp
class value_container {
public:
    // New API (Result<T>)
    Result<void> deserialize(const std::string& data_string,
                             bool parse_only_header = true);

    // Legacy API (deprecated)
    [[deprecated("Use Result<void> deserialize() instead")]]
    bool deserialize_legacy(const std::string& data_string,
                            bool parse_only_header = true) {
        auto result = deserialize(data_string, parse_only_header);
        return result.is_ok();
    }
};
```

**Timeline**:
- Phase 3.1: Add new Result<T> APIs
- Phase 3.2: Deprecate old APIs
- Phase 4: Remove deprecated APIs (after 6 months)

---

## Notes on Existing Integration

container_system already has partial Result<T> integration:

```cpp
#ifdef CONTAINER_USE_COMMON_SYSTEM
VoidResult deserialize_result(const std::string& data_string,
                               bool parse_only_header = true);
#endif
```

**Migration Strategy**:
1. Make Result<T> the primary API (not optional)
2. Remove `CONTAINER_USE_COMMON_SYSTEM` conditionals
3. Rename `deserialize_result()` to `deserialize()` (deprecate old version)
4. Ensure all dependencies use common_system Phase 3

---

## References

- [common_system Error Codes](../../common_system/include/kcenon/common/error/error_codes.h)
- [Error Handling Guidelines](../../common_system/docs/ERROR_HANDLING.md)
- [Result<T> Implementation](../../common_system/include/kcenon/common/patterns/result.h)
- [Container System RAII Guidelines](RAII_GUIDELINES.md)

---

**Document Status**: Phase 3 Preparation Complete
**Next Action**: Begin implementation or await approval
**Maintainer**: container_system team
