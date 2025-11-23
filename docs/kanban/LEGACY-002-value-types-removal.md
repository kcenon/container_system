# LEGACY-002: Remove Value Types Implementation Files

## Metadata
- **ID**: LEGACY-002
- **Priority**: HIGH
- **Estimated Duration**: 1 day
- **Dependencies**: LEGACY-001
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [LEGACY_REMOVAL_PLAN.md](../advanced/LEGACY_REMOVAL_PLAN.md)

---

## Overview

### What are we changing?

Remove all legacy value type implementation files in the `values/` directory. These files are completely replaced by variant_value_v2 + factory functions.

**Files to remove**:
- `values/bool_value.h` / `values/bool_value.cpp`
- `values/numeric_value.h` (template header)
- `values/string_value.h` / `values/string_value.cpp`
- `values/bytes_value.h` / `values/bytes_value.cpp`
- `values/array_value.h` / `values/array_value.cpp`
- `values/container_value.h` / `values/container_value.cpp`

**Total LOC to remove**: ~2,500 lines
**Total files to remove**: 11

---

## Changes

### How will we implement this?

#### 1. Replacement Mapping Verification

| Legacy Class | Modern Replacement | Status |
|--------------|-------------------|--------|
| `bool_value` | `make_bool_value()` | Ready |
| `short_value` | `make_short_value()` | Ready |
| `int_value` | `make_int_value()` | Ready |
| `long_value` | `make_long_value()` | Ready |
| `float_value` | `make_float_value()` | Ready |
| `double_value` | `make_double_value()` | Ready |
| `string_value` | `make_string_value()` | Ready |
| `bytes_value` | `make_bytes_value()` | Ready |
| `array_value` | `make_array_value()` | Ready |
| `container_value` | `make_container_value()` | Ready |

#### 2. File Removal Order

**Phase 1**: Remove simple types
```bash
rm values/bool_value.h values/bool_value.cpp
rm values/string_value.h values/string_value.cpp
```

**Phase 2**: Remove numeric types
```bash
rm values/numeric_value.h
# Also remove numeric_value.tpp if exists
```

**Phase 3**: Remove complex types
```bash
rm values/bytes_value.h values/bytes_value.cpp
rm values/array_value.h values/array_value.cpp
rm values/container_value.h values/container_value.cpp
```

**Phase 4**: Remove directory
```bash
rmdir values/  # Remove empty directory
```

#### 3. CMakeLists.txt Update

```cmake
# Remove
set(VALUE_SOURCES  # Remove this entire section
    ${CMAKE_CURRENT_SOURCE_DIR}/values/bool_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/string_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/bytes_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/array_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/container_value.cpp
)

# Remove values/ from include paths
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core      # Keep (value_types.h)
    ${CMAKE_CURRENT_SOURCE_DIR}/internal  # Keep
    # ${CMAKE_CURRENT_SOURCE_DIR}/values  # Remove
)
```

#### 4. Compatibility Header (Optional)

Compatibility header for migration period:

```cpp
// values/compatibility.h (temporary - remove after 1 release)
#pragma once

#warning "values/*.h headers are deprecated. Use internal/variant_value_factory.h"

#include "internal/variant_value_factory.h"

namespace container_module {

// Type aliases for backward compatibility
// These aliases will be removed in the next release

template<typename T>
[[deprecated("Use make_numeric_value<T>() instead")]]
inline variant_value_v2 create_numeric_value(const std::string& name, T value) {
    return make_numeric_value(name, value);
}

// ... other compatibility functions

}
```

---

## Test Plan

### How will we test this?

#### 1. Build Verification

```bash
# Check for include errors
cmake --build build 2>&1 | grep -E "values/.*\.h.*No such file"

# Check for symbol errors
cmake --build build 2>&1 | grep -E "undefined reference.*_value"
```

**Success criteria**: 0 errors

#### 2. Functionality Verification

```cpp
TEST(ValueTypesRemoval, AllTypesAvailable) {
    // All types can be created through factory
    auto bool_v = make_bool_value("b", true);
    auto short_v = make_short_value("s", int16_t{42});
    auto int_v = make_int_value("i", 42);
    auto long_v = make_long_value("l", int64_t{42});
    auto float_v = make_float_value("f", 3.14f);
    auto double_v = make_double_value("d", 3.14159);
    auto string_v = make_string_value("str", "test");
    auto bytes_v = make_bytes_value("bytes", std::vector<uint8_t>{0x01, 0x02});
    auto array_v = make_array_value("arr", std::vector<variant_value_v2>{});

    // Verify all types
    EXPECT_EQ(bool_v.type(), value_types::bool_value);
    EXPECT_EQ(short_v.type(), value_types::short_value);
    EXPECT_EQ(int_v.type(), value_types::int_value);
    EXPECT_EQ(long_v.type(), value_types::long_value);
    EXPECT_EQ(float_v.type(), value_types::float_value);
    EXPECT_EQ(double_v.type(), value_types::double_value);
    EXPECT_EQ(string_v.type(), value_types::string_value);
    EXPECT_EQ(bytes_v.type(), value_types::bytes_value);
    EXPECT_EQ(array_v.type(), value_types::array_value);
}
```

#### 3. Serialization Compatibility

```cpp
TEST(ValueTypesRemoval, SerializationCompatibility) {
    // Verify values created with new system serialize correctly
    auto val = make_int_value("test", 12345);
    auto bytes = val.serialize();

    // Verify expected format
    EXPECT_GT(bytes.size(), 0);

    // Round-trip
    auto restored = variant_value_v2::deserialize(bytes);
    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->get<int32_t>().value(), 12345);
}
```

#### Success Criteria
- [ ] 0 build errors
- [ ] All tests pass
- [ ] Serialization compatibility maintained

---

## Metrics

### Before/After Comparison

| Item | Before Removal | After Removal | Change |
|------|----------------|---------------|--------|
| Source file count | 58 | 47 | -11 |
| LOC (values/) | ~2,500 | 0 | -2,500 |
| Header file count | 28 | 22 | -6 |
| Build time | TBD | TBD | Expected -10% |

---

## Checklist

- [ ] Remove bool_value.h/cpp
- [ ] Remove numeric_value.h
- [ ] Remove string_value.h/cpp
- [ ] Remove bytes_value.h/cpp
- [ ] Remove array_value.h/cpp
- [ ] Remove container_value.h/cpp
- [ ] Remove values/ directory
- [ ] Update CMakeLists.txt
- [ ] Build verification
- [ ] Tests pass
- [ ] Serialization compatibility verified

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
