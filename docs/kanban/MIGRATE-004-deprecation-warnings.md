# MIGRATE-004: Add Legacy Classes Deprecation Warnings

## Metadata
- **ID**: MIGRATE-004
- **Priority**: LOW
- **Estimated Duration**: 0.5 days
- **Dependencies**: MIGRATE-001, MIGRATE-002, MIGRATE-003
- **Status**: DONE
- **Assignee**: TBD
- **Related Documents**: [MIGRATION_PLAN.md](../advanced/MIGRATION_PLAN.md)

---

## Overview

### What are we changing?

Add `[[deprecated]]` attributes to legacy value classes (`bool_value`, `int_value`, `string_value`, etc.) to guide users to migrate to the new API.

**Current state**:
- Legacy classes can be used without warnings
- Users are unaware of migration necessity

**Goals**:
- Display deprecation warnings at compile time
- Provide clear migration guide messages
- Encourage gradual migration

---

## Changes

### How will we implement this?

#### 1. Add [[deprecated]] to Legacy Classes

**values/bool_value.h**:
```cpp
namespace container_module {

[[deprecated("Use variant_value_v2 with make_bool_value() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class bool_value : public value {
    // ... existing implementation
};

}  // namespace container_module
```

**values/numeric_value.h**:
```cpp
template<typename T, value_types Type>
[[deprecated("Use variant_value_v2 with make_numeric_value<T>() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class numeric_value : public value {
    // ... existing implementation
};

// Also apply to type aliases
using short_value [[deprecated("Use make_short_value() instead")]]
    = numeric_value<int16_t, value_types::short_value>;
using int_value [[deprecated("Use make_int_value() instead")]]
    = numeric_value<int32_t, value_types::int_value>;
// ... other types
```

**values/string_value.h**:
```cpp
[[deprecated("Use variant_value_v2 with make_string_value() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class string_value : public value {
    // ... existing implementation
};
```

**values/bytes_value.h**:
```cpp
[[deprecated("Use variant_value_v2 with make_bytes_value() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class bytes_value : public value {
    // ... existing implementation
};
```

**values/array_value.h**:
```cpp
[[deprecated("Use variant_value_v2 with make_array_value() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class array_value : public value {
    // ... existing implementation
};
```

**values/container_value.h**:
```cpp
[[deprecated("Use thread_safe_container with make_container_value() instead. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class container_value : public value {
    // ... existing implementation
};
```

#### 2. Add Warning to value Base Class

**core/value.h**:
```cpp
[[deprecated("Polymorphic value system is deprecated. "
             "Use variant_value_v2 for new code. "
             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
class value {
    // ... existing implementation
};
```

#### 3. Add Warning to Legacy container.add() Method

**core/container.h**:
```cpp
class value_container {
public:
    [[deprecated("Use set_value() or set_unit() instead")]]
    void add(std::shared_ptr<value> val);

    // New recommended API
    template<typename T>
    void set_value(const std::string& key, T value);
};
```

#### 4. Provide Warning Suppression Macros

**core/deprecation.h**:
```cpp
#pragma once

// For cases where temporary warning suppression is needed during migration
#define CONTAINER_SUPPRESS_DEPRECATION_START \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#define CONTAINER_SUPPRESS_DEPRECATION_END \
    _Pragma("GCC diagnostic pop")

// MSVC support
#ifdef _MSC_VER
#undef CONTAINER_SUPPRESS_DEPRECATION_START
#undef CONTAINER_SUPPRESS_DEPRECATION_END
#define CONTAINER_SUPPRESS_DEPRECATION_START \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4996))
#define CONTAINER_SUPPRESS_DEPRECATION_END \
    __pragma(warning(pop))
#endif

// Usage example:
// CONTAINER_SUPPRESS_DEPRECATION_START
// auto val = std::make_shared<int_value>("key", 42);  // Warning suppressed
// CONTAINER_SUPPRESS_DEPRECATION_END
```

---

## Test Plan

### How will we test this?

#### 1. Verify Warning Generation

```cpp
// This code should generate deprecation warnings at compile time
void test_deprecation_warning() {
    auto val = std::make_shared<int_value>("count", 42);  // Warning!
}
```

**Expected output**:
```
warning: 'int_value' is deprecated: Use make_int_value() instead
```

#### 2. Test Warning Suppression Macros

```cpp
TEST(Deprecation, SuppressMacroWorks) {
    // No warnings should be generated in this block
    CONTAINER_SUPPRESS_DEPRECATION_START
    auto val = std::make_shared<int_value>("count", 42);
    CONTAINER_SUPPRESS_DEPRECATION_END

    EXPECT_NE(val, nullptr);
}
```

#### 3. Verify No Warning with New API

```cpp
TEST(ModernAPI, NoDeprecationWarning) {
    // New API should be usable without warnings
    auto val = make_int_value("count", 42);  // No warning
    EXPECT_EQ(val.type(), value_types::int_value);
}
```

#### Success Criteria
- [ ] Deprecation warnings added to all legacy classes
- [ ] Warning messages include migration guide
- [ ] Warning suppression macros work correctly
- [ ] No warnings when using new API

---

## Warning Message Guidelines

### Good Examples

```cpp
[[deprecated("Use make_int_value() instead. "
             "Migration guide: docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
```

- Clearly states what to use instead
- Includes migration document path
- Concise yet informative

### Bad Examples

```cpp
[[deprecated("Deprecated")]]  // Insufficient information

[[deprecated("This class will be removed in a future version due to "
             "performance concerns and type safety issues...")]]  // Too long
```

---

## Checklist

- [x] Add [[deprecated]] to bool_value (N/A - file does not exist)
- [x] Add [[deprecated]] to numeric_value (int_value, llong_value, etc.) (N/A - file does not exist)
- [x] Add [[deprecated]] to string_value (N/A - file does not exist)
- [x] Add [[deprecated]] to bytes_value (N/A - file does not exist)
- [x] Add [[deprecated]] to array_value (N/A - integrated into variant system)
- [x] Add [[deprecated]] to container_value (N/A - integrated into variant system)
- [x] Add [[deprecated]] to value base class (N/A - internal/value.h is new variant-based system)
- [x] Add [[deprecated]] to container.add() (completed at core/container.h:246)
- [x] Create deprecation.h macros (completed at core/deprecation.h)
- [x] Test warning generation
- [x] Test warning suppression (in test fixtures)
- [x] Update documentation

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-25
**Completed**: 2025-11-25

**Note**: Legacy value class files (bool_value, int_value, etc.) do not exist as separate files.
They have been integrated into the variant-based system (internal/value.h). Only the legacy
container.add() method remains and has been marked as deprecated.
