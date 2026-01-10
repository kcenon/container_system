# Result Pattern Migration Guide

This guide helps you migrate from the traditional exception/bool-based APIs to the new Result-returning APIs in container_system.

## Overview

Starting from version 2.0.0, container_system provides Result-based APIs that offer:

- **Type-safe error handling**: Errors are encoded in the return type
- **No exceptions at API boundaries**: Easier integration with C code and other languages
- **Detailed error information**: Error codes, messages, and source information
- **Composable operations**: Chain operations with `and_then()`, `or_else()`, etc.

## Error Codes

All error codes are defined in `container_module::error_codes` namespace:

```cpp
#include <container/container.h>

// Error categories:
// - 1xx: Value operations (key_not_found, type_mismatch, etc.)
// - 2xx: Serialization (serialization_failed, deserialization_failed, etc.)
// - 3xx: Validation (schema_validation_failed, constraint_violated, etc.)
// - 4xx: Resource (memory_allocation_failed, file_not_found, etc.)
// - 5xx: Thread safety (lock_acquisition_failed, concurrent_modification)
```

## Migration Examples

### Value Access

**Before (traditional API):**
```cpp
auto opt_value = container->get_value("key");
if (!opt_value) {
    // Handle error - but no error details available
    return;
}
auto value = *opt_value;
```

**After (Result API):**
```cpp
auto result = container->get_result("key");
if (!result.is_ok()) {
    auto& err = result.error();
    std::cerr << "Error " << err.code << ": " << err.message << std::endl;
    return;
}
auto value = result.value();
```

### Typed Value Access

**Before:**
```cpp
try {
    // get<T>() already uses Result, but you might use exception-based patterns
    auto result = container->get<int>("count");
    if (result.is_ok()) {
        int count = result.value();
    }
} catch (...) {
    // Handle error
}
```

**After (idiomatic usage):**
```cpp
auto result = container->get<int>("count");
if (!result.is_ok()) {
    if (result.error().code == error_codes::key_not_found) {
        // Use default value
        return 0;
    }
    // Log and propagate error
    return result;
}
return result.value();
```

### Setting Values

**Before (chaining API):**
```cpp
container->set("key1", 100)
         .set("key2", "value")
         .set("key3", 3.14);
// No error information available
```

**After (Result API):**
```cpp
auto r1 = container->set_result("key1", 100);
if (!r1.is_ok()) {
    return r1;  // Propagate error
}

auto r2 = container->set_result("key2", "value");
if (!r2.is_ok()) {
    return r2;
}

auto r3 = container->set_result("key3", 3.14);
if (!r3.is_ok()) {
    return r3;
}
```

**Alternative (bulk operation):**
```cpp
std::vector<optimized_value> values = {
    {"key1", value_types::int_value, 100},
    {"key2", value_types::string_value, std::string("value")},
    {"key3", value_types::double_value, 3.14}
};
auto result = container->set_all_result(values);
if (!result.is_ok()) {
    return result;
}
```

### Serialization

**Before:**
```cpp
try {
    std::string data = container->serialize();
    // Use data
} catch (const std::exception& e) {
    std::cerr << "Serialization failed: " << e.what() << std::endl;
}
```

**After:**
```cpp
auto result = container->serialize_result();
if (!result.is_ok()) {
    auto& err = result.error();
    std::cerr << "Serialization failed (code " << err.code << "): "
              << err.message << std::endl;
    return;
}
std::string data = result.value();
```

### Deserialization

**Before:**
```cpp
if (!container->deserialize(data)) {
    // Failed, but no error details
    return false;
}
```

**After:**
```cpp
auto result = container->deserialize_result(data);
if (!result.is_ok()) {
    auto& err = result.error();
    if (error_codes::is_serialization_error(err.code)) {
        std::cerr << "Parse error: " << err.message << std::endl;
    }
    return result;
}
```

### File Operations

**Before:**
```cpp
try {
    container->load_packet("data.bin");
} catch (const std::exception& e) {
    std::cerr << "Failed to load: " << e.what() << std::endl;
}
```

**After:**
```cpp
auto result = container->load_packet_result("data.bin");
if (!result.is_ok()) {
    auto& err = result.error();
    if (err.code == error_codes::file_not_found) {
        std::cerr << "File not found: data.bin" << std::endl;
    } else if (err.code == error_codes::file_read_error) {
        std::cerr << "Read error: " << err.message << std::endl;
    }
    return;
}
```

### JSON/XML Conversion

**Before:**
```cpp
std::string json = container->to_json();  // May throw
std::string xml = container->to_xml();    // May throw
```

**After:**
```cpp
auto json_result = container->to_json_result();
if (json_result.is_ok()) {
    std::string json = json_result.value();
}

auto xml_result = container->to_xml_result();
if (xml_result.is_ok()) {
    std::string xml = xml_result.value();
}
```

## API Reference

### New Result-returning Methods

| Traditional API | Result API | Return Type |
|----------------|------------|-------------|
| `get_value(key)` | `get_result(key)` | `Result<optimized_value>` |
| `get<T>(key)` | (same) | `Result<T>` |
| `set(key, value)` | `set_result(key, value)` | `VoidResult` |
| `set(val)` | `set_result(val)` | `VoidResult` |
| `set_all(vals)` | `set_all_result(vals)` | `VoidResult` |
| `remove(key)` | `remove_result(key)` | `VoidResult` |
| `serialize()` | `serialize_result()` | `Result<string>` |
| `serialize_array()` | `serialize_array_result()` | `Result<vector<uint8_t>>` |
| `deserialize(data)` | `deserialize_result(data)` | `VoidResult` |
| `to_json()` | `to_json_result()` | `Result<string>` |
| `to_xml()` | `to_xml_result()` | `Result<string>` |
| `load_packet(path)` | `load_packet_result(path)` | `VoidResult` |
| `save_packet(path)` | `save_packet_result(path)` | `VoidResult` |

### Error Handling Utilities

```cpp
// Check error category
if (error_codes::is_value_error(err.code)) { /* 1xx errors */ }
if (error_codes::is_serialization_error(err.code)) { /* 2xx errors */ }
if (error_codes::is_validation_error(err.code)) { /* 3xx errors */ }
if (error_codes::is_resource_error(err.code)) { /* 4xx errors */ }
if (error_codes::is_thread_error(err.code)) { /* 5xx errors */ }

// Get error message
std::string_view msg = error_codes::get_message(err.code);

// Get error category name
std::string_view category = error_codes::get_category(err.code);

// Build detailed message
std::string detailed = error_codes::make_message(err.code, "context");
```

## Best Practices

1. **Prefer Result APIs for new code**: They provide better error information and are more explicit about failure modes.

2. **Check errors explicitly**: Don't ignore return values - all Result-returning methods are marked `[[nodiscard]]`.

3. **Use error codes for decisions**: Switch on error codes rather than parsing error messages.

4. **Propagate errors up**: Use early returns to propagate errors to callers who can handle them.

5. **Log error details**: Include error code, message, and source in logs for debugging.

## Deprecation Timeline

- **v2.0.0**: Result APIs available alongside traditional APIs
- **v2.1.0**: Traditional APIs will be marked `[[deprecated]]`
- **v3.0.0**: Traditional APIs may be removed

## Requirements

- **Compiler**: C++20 or later
- **Dependencies**: Requires `common_system` with Result<T> support
- **Macro**: `KCENON_HAS_COMMON_SYSTEM` must be defined

## See Also

- [Error Codes Reference](../API_REFERENCE.md#error-codes)
- [Result<T> Pattern Documentation](https://github.com/kcenon/common_system/docs/result.md)
