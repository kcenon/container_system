# MIGRATE-001: Implement variant_value_v2 Factory Functions

## Metadata
- **ID**: MIGRATE-001
- **Priority**: HIGH
- **Estimated Duration**: 1 day
- **Dependencies**: None
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [MIGRATION_PLAN.md](../advanced/MIGRATION_PLAN.md)

---

## Overview

### What are we changing?

Implement type-safe factory functions for the variant_value_v2 system, allowing users to easily create values with the new system.

**Current state**:
- variant_value_v2 core implementation complete
- Factory functions not implemented

**Goals**:
- Provide per-type factory functions
- Maintain similar usage experience to legacy API
- Ensure compile-time type safety

---

## Changes

### How will we implement this?

#### 1. Factory Function Definitions (variant_value_factory.h)

```cpp
#pragma once

#include "variant_value_v2.h"
#include <memory>
#include <string>

namespace container_module {

// Boolean factory
inline variant_value_v2 make_bool_value(const std::string& name, bool value) {
    return variant_value_v2(name, value);
}

// Numeric factories
template<typename T>
inline variant_value_v2 make_numeric_value(const std::string& name, T value) {
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");
    return variant_value_v2(name, value);
}

// Convenience numeric factories
inline variant_value_v2 make_short_value(const std::string& name, int16_t value) {
    return variant_value_v2(name, value);
}

inline variant_value_v2 make_int_value(const std::string& name, int32_t value) {
    return variant_value_v2(name, value);
}

inline variant_value_v2 make_long_value(const std::string& name, int64_t value) {
    return variant_value_v2(name, value);
}

inline variant_value_v2 make_float_value(const std::string& name, float value) {
    return variant_value_v2(name, value);
}

inline variant_value_v2 make_double_value(const std::string& name, double value) {
    return variant_value_v2(name, value);
}

// String factory
inline variant_value_v2 make_string_value(const std::string& name,
                                          const std::string& value) {
    return variant_value_v2(name, value);
}

inline variant_value_v2 make_string_value(const std::string& name,
                                          std::string&& value) {
    return variant_value_v2(name, std::move(value));
}

// Bytes factory
inline variant_value_v2 make_bytes_value(const std::string& name,
                                         const std::vector<uint8_t>& data) {
    return variant_value_v2(name, data);
}

inline variant_value_v2 make_bytes_value(const std::string& name,
                                         std::vector<uint8_t>&& data) {
    return variant_value_v2(name, std::move(data));
}

// Array factory
inline variant_value_v2 make_array_value(const std::string& name,
                                         const std::vector<variant_value_v2>& values) {
    return variant_value_v2(name, values);
}

// Container factory
inline variant_value_v2 make_container_value(
    const std::string& name,
    std::shared_ptr<thread_safe_container> container) {
    return variant_value_v2(name, std::move(container));
}

}  // namespace container_module
```

#### 2. Header File Location

**File path**: `internal/variant_value_factory.h`

#### 3. CMakeLists.txt Update

```cmake
set(INTERNAL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_v2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_factory.h  # Add
    # ...
)
```

---

## Test Plan

### How will we test this?

#### 1. Unit Tests

```cpp
TEST(VariantValueFactory, MakeBoolValue) {
    auto val = make_bool_value("flag", true);
    EXPECT_EQ(val.name(), "flag");
    EXPECT_EQ(val.type(), value_types::bool_value);
    EXPECT_TRUE(val.get<bool>().value());
}

TEST(VariantValueFactory, MakeIntValue) {
    auto val = make_int_value("count", 42);
    EXPECT_EQ(val.type(), value_types::int_value);
    EXPECT_EQ(val.get<int32_t>().value(), 42);
}

TEST(VariantValueFactory, MakeStringValue) {
    auto val = make_string_value("name", "test");
    EXPECT_EQ(val.type(), value_types::string_value);
    EXPECT_EQ(val.get<std::string>().value(), "test");
}

TEST(VariantValueFactory, MakeBytesValue) {
    std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
    auto val = make_bytes_value("data", data);
    EXPECT_EQ(val.type(), value_types::bytes_value);
    EXPECT_EQ(val.get<std::vector<uint8_t>>().value(), data);
}

TEST(VariantValueFactory, MakeArrayValue) {
    std::vector<variant_value_v2> items = {
        make_int_value("a", 1),
        make_int_value("b", 2),
        make_int_value("c", 3)
    };
    auto val = make_array_value("numbers", items);
    EXPECT_EQ(val.type(), value_types::array_value);
}
```

#### 2. Compile Test (Type Safety)

```cpp
// This should cause a compile error
// make_numeric_value("invalid", "string");  // static_assert failure
```

#### 3. Performance Test

```cpp
TEST(VariantValueFactory, Performance) {
    const int iterations = 1'000'000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto val = make_int_value("count", i);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Factory creation: " << iterations / duration.count() << "K ops/ms\n";

    // Target: 1M ops/sec or higher
    EXPECT_GT(iterations / duration.count(), 1000);
}
```

#### Success Criteria
- [ ] Factory functions implemented for all types
- [ ] Unit tests 100% pass
- [ ] Performance target achieved (1M ops/sec)
- [ ] Documentation complete

---

## Usage Examples

### Before (Legacy)
```cpp
auto val = std::make_shared<int_value>("count", 42);
container.add(val);
```

### After (Modern)
```cpp
auto val = make_int_value("count", 42);
container.set_optimized_value(val.to_optimized());
// Or directly
container.set_value("count", 42);
```

---

## Checklist

- [ ] Create `variant_value_factory.h`
- [ ] Implement `make_bool_value()`
- [ ] Implement `make_numeric_value<T>()`
- [ ] Implement `make_string_value()`
- [ ] Implement `make_bytes_value()`
- [ ] Implement `make_array_value()`
- [ ] Implement `make_container_value()`
- [ ] Update CMakeLists.txt
- [ ] Write and pass unit tests
- [ ] Pass performance tests

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
