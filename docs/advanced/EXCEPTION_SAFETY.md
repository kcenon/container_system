# Exception Safety Guarantees

**Date**: 2025-11-09
**Status**: Active

## Overview

This document defines the exception safety guarantees for all public APIs in container_system.
Understanding these guarantees helps users write robust error-handling code.

## Exception Safety Levels

We follow the standard C++ exception safety guarantees:

### 1. No-throw Guarantee (noexcept)
- **Definition**: Operation will never throw an exception
- **Promise**: The operation always succeeds
- **Indication**: Marked with `noexcept` specifier

### 2. Strong Guarantee
- **Definition**: If an exception is thrown, program state is rolled back to the state before the operation
- **Promise**: Either complete success or no change at all (transactional)
- **Examples**: Operations with rollback capability

### 3. Basic Guarantee
- **Definition**: If an exception is thrown, the object remains in a valid state
- **Promise**: No resource leaks, invariants maintained, but object state may be changed
- **Examples**: Operations that partially modify state before failing

### 4. No Guarantee
- **Definition**: No promises made about exception behavior
- **Note**: We avoid this in public APIs

---

## API Exception Safety Reference

### Core Container APIs

#### Serialization Methods

```cpp
/**
 * @brief Serialize container to JSON string
 * @exception_safety Strong guarantee - no changes on exception
 * @throws std::bad_alloc if memory allocation fails
 * @throws std::runtime_error if value cannot be serialized
 */
std::string serialize() const;

/**
 * @brief Serialize container to byte array
 * @exception_safety Strong guarantee - no changes on exception
 * @throws std::bad_alloc if memory allocation fails
 * @throws std::runtime_error if value cannot be serialized
 */
std::vector<uint8_t> serialize_array() const;
```

#### Deserialization Methods

```cpp
/**
 * @brief Deserialize from string (bool return)
 * @exception_safety Basic guarantee - container may be partially modified
 * @throws None - returns false on failure instead
 * @return true on success, false on parse error
 */
bool deserialize(const std::string& data_string, bool parse_only_header = true);

/**
 * @brief Deserialize from byte array (bool return)
 * @exception_safety Basic guarantee - container may be partially modified
 * @throws None - returns false on failure instead
 * @return true on success, false on parse error
 */
bool deserialize(const std::vector<uint8_t>& data_array, bool parse_only_header = true);
```

#### Result-based Alternatives (Recommended)

```cpp
#if KCENON_HAS_COMMON_SYSTEM
/**
 * @brief Deserialize with detailed error information
 * @exception_safety Strong guarantee - no changes on error
 * @throws None - errors returned in Result object
 * @return Result containing success/error with context
 */
kcenon::common::VoidResult deserialize_result(
    const std::string& data_string,
    bool parse_only_header = true) noexcept;

/**
 * @brief Deserialize from bytes with detailed error information
 * @exception_safety Strong guarantee - no changes on error
 * @throws None - errors returned in Result object
 * @return Result containing success/error with context
 */
kcenon::common::VoidResult deserialize_result(
    const std::vector<uint8_t>& data_array,
    bool parse_only_header = true) noexcept;
#endif
```

### Value Construction

#### Numeric Values

```cpp
/**
 * @brief Construct numeric value with range checking
 * @exception_safety Strong guarantee - object not created on exception
 * @throws std::overflow_error if value exceeds type's valid range
 * @note For long_value: enforces 32-bit range [-2147483648, 2147483647]
 */
template <typename T>
numeric_value(const std::string& name, const T& initial_value);
```

**Recommended Alternative**:
```cpp
/**
 * @brief Create numeric value with range validation (no exceptions)
 * @exception_safety No-throw guarantee
 * @return Result<shared_ptr<numeric_value<T>>> containing value or error
 */
template <typename T>
static Result<std::shared_ptr<numeric_value<T>>> create_checked(
    const std::string& name,
    const T& initial_value) noexcept;
```

#### Array Values

```cpp
/**
 * @brief Deserialize array from binary data
 * @exception_safety Basic guarantee - may partially parse array
 * @throws std::runtime_error if data is malformed
 * @throws std::bad_alloc if memory allocation fails
 */
static std::shared_ptr<array_value> deserialize(const std::vector<uint8_t>& byte_vector);
```

### Container Operations

#### Value Access

```cpp
/**
 * @brief Get value by key
 * @exception_safety No-throw guarantee
 * @throws None
 * @return Shared pointer (null if not found)
 */
std::shared_ptr<value> get_value(std::string_view target_name, unsigned int index = 0) noexcept;
```

#### Value Modification

```cpp
/**
 * @brief Add or update value
 * @exception_safety Strong guarantee - no change on failure
 * @throws std::bad_alloc if memory allocation fails
 */
void add(const std::string& key, std::shared_ptr<value> val);

/**
 * @brief Remove value by key
 * @exception_safety No-throw guarantee
 * @throws None
 * @return true if removed, false if not found
 */
bool remove(std::string_view key) noexcept;
```

---

## Migration Guide: Exception-throwing to Result-based

For code that needs exception safety without try-catch overhead, use Result-based alternatives:

### Before (Exception-based)
```cpp
try {
    auto container = std::make_shared<value_container>();
    container->deserialize(json_data);
    // Use container
} catch (const std::exception& e) {
    std::cerr << "Deserialization failed: " << e.what() << std::endl;
}
```

### After (Result-based)
```cpp
auto container = std::make_shared<value_container>();
auto result = container->deserialize_result(json_data);

if (result.is_error()) {
    std::cerr << "Deserialization failed: "
              << result.error().message() << std::endl;
    return;
}

// Use container
```

### Benefits of Result-based APIs
1. **No hidden control flow** - errors are explicit return values
2. **Better performance** - no stack unwinding overhead
3. **Composable** - can chain operations with `and_then()`, `or_else()`
4. **Detailed context** - error objects carry more information than exceptions

---

## Implementation Guidelines

For developers adding new APIs:

### 1. Default to noexcept when possible
```cpp
bool is_empty() const noexcept;
size_t size() const noexcept;
void clear() noexcept;
```

### 2. Document exception behavior
```cpp
/**
 * @exception_safety Strong guarantee
 * @throws std::invalid_argument if key is empty
 * @throws std::bad_alloc if allocation fails
 */
void add_value(const std::string& key, const std::string& value);
```

### 3. Provide Result<T> alternatives for throwing operations
```cpp
// Throwing version
void parse(const std::string& data);

// Non-throwing alternative
VoidResult parse_safe(const std::string& data) noexcept;
```

### 4. Use RAII for resource management
```cpp
class scoped_lock {
    std::mutex& mtx_;
public:
    scoped_lock(std::mutex& m) : mtx_(m) { mtx_.lock(); }
    ~scoped_lock() noexcept { mtx_.unlock(); }
};
```

---

## Testing Exception Safety

All exception-throwing code should have corresponding tests:

```cpp
TEST(ExceptionSafety, SerializeBadAlloc) {
    // Test that strong guarantee is maintained
    value_container original;
    // ... populate ...

    // Inject allocation failure
    try {
        std::string serialized = original.serialize();
    } catch (const std::bad_alloc&) {
        // Verify original container unchanged
        EXPECT_EQ(original.size(), expected_size);
    }
}

TEST(ExceptionSafety, NumericValueOverflow) {
    // Verify that overflow is detected
    EXPECT_THROW({
        numeric_value<long> val("test", LLONG_MAX);
    }, std::overflow_error);
}

TEST(ResultSafety, DeserializeInvalid) {
    // Verify Result-based API never throws
    value_container container;
    auto result = container.deserialize_result("invalid json");

    EXPECT_TRUE(result.is_error());
    EXPECT_FALSE(result.is_ok());
    // Container should be unchanged
}
```

---

## Known Issues and Future Work

### Current Limitations

1. **Partial Deserialization**: `deserialize()` has basic guarantee only
   - Future: Implement transactional deserialization with rollback

2. **Array Operations**: Some array methods throw on malformed data
   - Future: Provide Result<T> alternatives

3. **Mixed Exception Models**: Some old code uses bool returns, some throws
   - Future: Standardize on Result<T> for all fallible operations

---

## References

- [C++ Core Guidelines: E.12](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#e12-use-noexcept-when-exiting-a-function-because-of-a-throw-is-impossible-or-unacceptable)
- [C++ Core Guidelines: E.15](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#e15-throw-by-value-catch-exceptions-from-a-hierarchy-by-reference)
- Project: `common_system/docs/RESULT_PATTERN.md` (for Result<T> usage)

---

**Last Updated**: 2025-11-09
**Review Cycle**: Quarterly
