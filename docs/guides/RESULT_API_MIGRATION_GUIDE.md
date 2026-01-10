# Result API Migration Guide

> **Status**: Ready for Production
> **Version**: 2.0.0
> **Last Updated**: 2025-01-11
>
> **IMPORTANT**: As of this version, the void/bool returning methods are **DEPRECATED**
> and will be removed in the next major version. Please migrate to Result-returning APIs.

---

## Deprecation Notice

### What's Deprecated?

Starting from this version, the following methods are marked as `[[deprecated]]` and will trigger compiler warnings:

| Deprecated Method | Replacement Method |
|------------------|-------------------|
| `serialize()` | `serialize_result()` |
| `serialize_array()` | `serialize_array_result()` |
| `deserialize()` | `deserialize_result()` |
| `to_json()` | `to_json_result()` |
| `to_xml()` | `to_xml_result()` |
| `to_msgpack()` | `to_msgpack_result()` |
| `from_msgpack()` | `from_msgpack_result()` |
| `load_packet()` | `load_packet_result()` |
| `save_packet()` | `save_packet_result()` |
| `remove()` | `remove_result()` |

### Timeline

| Date | Version | Action |
|------|---------|--------|
| **2025-01-11** | v2.0.0 | Deprecation warnings added |
| **TBD** | v2.x.x | Last version supporting deprecated methods |
| **TBD** | v3.0.0 | Deprecated methods removed, `_result` suffix removed |

**Recommendation**: Complete migration to benefit from proper error handling.

---

## Table of Contents

- [Executive Summary](#executive-summary)
- [Why Migrate?](#why-migrate)
- [Migration Strategy](#migration-strategy)
- [API Comparison](#api-comparison)
- [Step-by-Step Migration](#step-by-step-migration)
- [Error Handling Patterns](#error-handling-patterns)
- [Testing Strategy](#testing-strategy)
- [FAQ](#faq)

---

## Executive Summary

The Result-based API provides:

- **Type-safe error handling** without exceptions at API boundaries
- **Detailed error information** with error codes and messages
- **Consistent API pattern** across all operations
- **No-throw guarantee** for noexcept methods
- **Better integration** with C code and other languages

### Benefits

| Aspect | Old API | Result API |
|--------|---------|------------|
| Error context | None (bool) or exception | Error code + message |
| Exception safety | May throw | noexcept (no-throw) |
| Error propagation | Manual | Built-in chaining |
| Type safety | Weak | Strong |

---

## Why Migrate?

### Problems with Legacy API

**Problem 1: Lost Error Context**

```cpp
// OLD: No information about why it failed
if (!container->deserialize(data)) {
    // What went wrong? Invalid format? Corrupted data? Version mismatch?
    std::cerr << "Deserialization failed" << std::endl;
}
```

**Problem 2: Exception Handling Required**

```cpp
// OLD: Must catch exceptions
try {
    auto result = container->serialize();
    // Use result...
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

**Problem 3: No Standardized Error Codes**

```cpp
// OLD: Ad-hoc error handling
if (!container->deserialize(data)) {
    // Is it safe to retry? Is the data corrupted?
    // No way to programmatically distinguish error types
}
```

### Solution: Result-based API

```cpp
// NEW: Rich error information
auto result = container->deserialize_result(data);
if (!result.is_ok()) {
    auto& err = result.error();
    switch (err.code) {
        case error_codes::corrupted_data:
            // Data is corrupted, don't retry
            break;
        case error_codes::version_mismatch:
            // Try legacy format
            break;
        default:
            log_error("Error {}: {}", err.code, err.message);
    }
}
```

---

## Migration Strategy

### Phase 1: Suppress Warnings (Optional)

If you need time before migrating, you can suppress deprecation warnings:

```cpp
// Suppress warning for a single call
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
container->serialize();  // No warning
#pragma GCC diagnostic pop
```

**Note**: This is a temporary measure. We recommend migrating as soon as possible.

### Phase 2: Incremental Migration

Migrate one method at a time:

```cpp
// Before
std::string data = container->serialize();

// After
auto result = container->serialize_result();
if (result.is_ok()) {
    std::string data = result.value();
} else {
    // Handle error
}
```

### Phase 3: Complete Migration

After all methods are migrated, your code will be warning-free and benefit from proper error handling.

---

## API Comparison

### Serialization

```cpp
// OLD
std::string serialize();
std::vector<uint8_t> serialize_array();

// NEW
[[nodiscard]] Result<std::string> serialize_result() noexcept;
[[nodiscard]] Result<std::vector<uint8_t>> serialize_array_result() noexcept;
```

### Deserialization

```cpp
// OLD
bool deserialize(const std::string& data, bool parse_only_header = true);
bool deserialize(const std::vector<uint8_t>& data, bool parse_only_header = true);

// NEW
[[nodiscard]] VoidResult deserialize_result(const std::string& data,
                                            bool parse_only_header = true) noexcept;
[[nodiscard]] VoidResult deserialize_result(const std::vector<uint8_t>& data,
                                            bool parse_only_header = true) noexcept;
```

### Value Operations

```cpp
// OLD (these were already migrated)
template<typename T>
void set_value(const std::string& key, T&& value);  // DEPRECATED
void remove(std::string_view key, bool update_immediately = false);

// NEW
template<typename T>
[[nodiscard]] VoidResult set_result(std::string_view key, T&& value) noexcept;
[[nodiscard]] Result<T> get(std::string_view key) noexcept;  // Already uses Result
[[nodiscard]] VoidResult remove_result(std::string_view key) noexcept;
```

### File Operations

```cpp
// OLD
void load_packet(const std::string& path);  // Throws on error
void save_packet(const std::string& path);  // Throws on error

// NEW
[[nodiscard]] VoidResult load_packet_result(const std::string& path) noexcept;
[[nodiscard]] VoidResult save_packet_result(const std::string& path) noexcept;
```

### Format Conversion

```cpp
// OLD
const std::string to_json();
const std::string to_xml();
std::vector<uint8_t> to_msgpack();
bool from_msgpack(const std::vector<uint8_t>& data);

// NEW
[[nodiscard]] Result<std::string> to_json_result() noexcept;
[[nodiscard]] Result<std::string> to_xml_result() noexcept;
[[nodiscard]] Result<std::vector<uint8_t>> to_msgpack_result() noexcept;
[[nodiscard]] VoidResult from_msgpack_result(const std::vector<uint8_t>& data) noexcept;
```

---

## Step-by-Step Migration

### Example 1: Serialization

```cpp
// BEFORE
void save_data(std::shared_ptr<value_container> container) {
    try {
        std::string data = container->serialize();
        write_to_file(data);
    } catch (const std::exception& e) {
        std::cerr << "Failed to serialize: " << e.what() << std::endl;
    }
}

// AFTER
void save_data(std::shared_ptr<value_container> container) {
    auto result = container->serialize_result();
    if (result.is_ok()) {
        write_to_file(result.value());
    } else {
        std::cerr << "Failed to serialize: " << result.error().message << std::endl;
    }
}
```

### Example 2: Deserialization

```cpp
// BEFORE
bool load_data(std::shared_ptr<value_container> container, const std::string& data) {
    if (!container->deserialize(data)) {
        std::cerr << "Failed to deserialize" << std::endl;
        return false;
    }
    return true;
}

// AFTER
bool load_data(std::shared_ptr<value_container> container, const std::string& data) {
    auto result = container->deserialize_result(data);
    if (!result.is_ok()) {
        auto& err = result.error();
        std::cerr << "Failed to deserialize (code " << err.code << "): "
                  << err.message << std::endl;
        return false;
    }
    return true;
}
```

### Example 3: File Operations

```cpp
// BEFORE
void process_file(std::shared_ptr<value_container> container, const std::string& path) {
    try {
        container->load_packet(path);
        // Process...
        container->save_packet(path + ".out");
    } catch (const std::exception& e) {
        std::cerr << "File operation failed: " << e.what() << std::endl;
    }
}

// AFTER
void process_file(std::shared_ptr<value_container> container, const std::string& path) {
    if (auto result = container->load_packet_result(path); !result.is_ok()) {
        std::cerr << "Failed to load: " << result.error().message << std::endl;
        return;
    }

    // Process...

    if (auto result = container->save_packet_result(path + ".out"); !result.is_ok()) {
        std::cerr << "Failed to save: " << result.error().message << std::endl;
    }
}
```

---

## Error Handling Patterns

### Pattern 1: Simple Check

```cpp
auto result = container->serialize_result();
if (result.is_ok()) {
    use_data(result.value());
} else {
    handle_error(result.error());
}
```

### Pattern 2: Error Code Switch

```cpp
auto result = container->deserialize_result(data);
if (!result.is_ok()) {
    switch (result.error().code) {
        case error_codes::corrupted_data:
            log_warning("Data corruption detected");
            break;
        case error_codes::version_mismatch:
            return try_legacy_format(data);
        case error_codes::invalid_format:
            return use_default_container();
        default:
            throw std::runtime_error(result.error().message);
    }
}
```

### Pattern 3: Early Return

```cpp
VoidResult process(std::shared_ptr<value_container> container, const std::string& data) {
    if (auto r = container->deserialize_result(data); !r.is_ok()) {
        return r;  // Propagate error
    }

    if (auto r = container->set_result("processed", true); !r.is_ok()) {
        return r;  // Propagate error
    }

    return container->serialize_result();  // Return final result
}
```

### Pattern 4: Value or Default

```cpp
// Get value or use default
auto name = container->get<std::string>("name");
std::string actual_name = name.is_ok() ? name.value() : "Unknown";

// Using value_or helper (if available)
std::string actual_name = kcenon::common::value_or(
    container->get<std::string>("name"),
    "Unknown"
);
```

---

## Error Codes Reference

| Code | Name | Description |
|------|------|-------------|
| 100 | `key_not_found` | Requested key does not exist |
| 101 | `type_mismatch` | Value type doesn't match requested type |
| 102 | `value_out_of_range` | Value exceeds valid range |
| 103 | `invalid_value` | Value is invalid |
| 104 | `empty_key` | Key is empty |
| 200 | `serialization_failed` | Serialization operation failed |
| 201 | `deserialization_failed` | Deserialization operation failed |
| 202 | `invalid_format` | Data format is invalid |
| 203 | `version_mismatch` | Version incompatibility |
| 204 | `corrupted_data` | Data is corrupted |
| 300 | `schema_validation_failed` | Schema validation failed |
| 301 | `missing_required_field` | Required field is missing |
| 302 | `constraint_violated` | Constraint was violated |
| 400 | `memory_allocation_failed` | Memory allocation failed |
| 401 | `file_not_found` | File does not exist |
| 402 | `file_read_error` | Error reading file |
| 403 | `file_write_error` | Error writing file |

---

## Testing Strategy

### Unit Tests

```cpp
TEST(ResultApiMigration, DeserializeErrorHandling) {
    auto container = std::make_shared<value_container>();

    // Test invalid data
    auto result = container->deserialize_result("invalid_data");
    ASSERT_FALSE(result.is_ok());
    EXPECT_EQ(result.error().code, error_codes::invalid_format);
}

TEST(ResultApiMigration, SerializeSuccess) {
    auto container = std::make_shared<value_container>();
    container->set("key", "value");

    auto result = container->serialize_result();
    ASSERT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
}
```

### Integration Tests

```cpp
TEST(ResultApiMigration, FileRoundTrip) {
    auto container = std::make_shared<value_container>();
    container->set("test", 42);

    // Save
    auto save_result = container->save_packet_result("/tmp/test.bin");
    ASSERT_TRUE(save_result.is_ok());

    // Load
    auto new_container = std::make_shared<value_container>();
    auto load_result = new_container->load_packet_result("/tmp/test.bin");
    ASSERT_TRUE(load_result.is_ok());

    // Verify
    auto value = new_container->get<int>("test");
    ASSERT_TRUE(value.is_ok());
    EXPECT_EQ(value.value(), 42);
}
```

---

## FAQ

### Q: Can I still use the old API?

**A**: Yes, but you will see deprecation warnings. The old API will be removed in version 3.0.0.

### Q: What if I have a large codebase?

**A**: Migrate incrementally. Start with the most critical paths (error handling, file I/O) and work outward.

### Q: Are the Result APIs thread-safe?

**A**: Yes, the Result-returning methods have the same thread-safety guarantees as their deprecated counterparts.

### Q: Why `_result` suffix instead of just replacing the old methods?

**A**: This allows for gradual migration without breaking existing code. In version 3.0.0, the `_result` suffix will be removed and these will become the primary APIs.

### Q: What happens to error handling for `set()` methods?

**A**: The unified `set()` API returns `value_container&` for chaining. Use `set_result()` if you need error information.

---

## Summary

1. **Update method calls** from deprecated methods to `_result` versions
2. **Handle errors** using `is_ok()` and `error()` methods
3. **Use error codes** for programmatic error handling
4. **Test thoroughly** before deploying

For questions or issues, please open a GitHub issue.
