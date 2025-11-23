# LEGACY-003: Remove Old variant_value

## Metadata
- **ID**: LEGACY-003
- **Priority**: MEDIUM
- **Estimated Duration**: 0.5 days
- **Dependencies**: LEGACY-001, LEGACY-002
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [LEGACY_REMOVAL_PLAN.md](../advanced/LEGACY_REMOVAL_PLAN.md)

---

## Overview

### What are we changing?

Remove the old `variant_value` implementation (LegacyValueVariant) in the `internal/` directory. It has been completely replaced by `variant_value_v2`.

**Files to remove**:
- `internal/variant_value.h` - Old variant implementation
- `internal/variant_value.cpp` - Old variant implementation

**Files to keep**:
- `internal/variant_value_v2.h` - New variant implementation
- `internal/variant_value_v2.cpp` - New variant implementation
- `internal/variant_value_factory.h` - Factory functions
- `internal/thread_safe_container.h` - Thread-safe container
- `internal/thread_safe_container.cpp`

---

## Changes

### How will we implement this?

#### 1. Problems with Old variant_value

**Type Index Mismatch (from VALUE_SYSTEM_COMPARISON_ANALYSIS.md)**:
```
value_types enum     variant<...> index     RESULT
─────────────────────────────────────────────────────
short_value   (2)  →  int16_t        [3]    ❌ MISMATCH +1
bytes_value  (12)  →  vector<u8>     [2]    🔴 CRITICAL: -10
string_value (13)  →  string        [11]    🔴 CRITICAL: -2
array_value  (15)  →  MISSING!             🔴 MISSING TYPE
```

This issue can cause data loss and must be removed.

#### 2. Search and Update References

```bash
# Search old variant_value usage
grep -r "variant_value\b" --include="*.cpp" --include="*.h" | grep -v "variant_value_v2"

# Search LegacyValueVariant usage
grep -r "LegacyValueVariant" --include="*.cpp" --include="*.h"
```

#### 3. File Removal

```bash
# Remove old variant_value files
rm internal/variant_value.h
rm internal/variant_value.cpp
```

#### 4. CMakeLists.txt Update

```cmake
set(INTERNAL_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.cpp  # Remove
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_v2.cpp  # Keep
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.cpp  # Keep
)

set(INTERNAL_HEADERS
    # ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.h  # Remove
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_v2.h  # Keep
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value_factory.h  # Keep
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.h  # Keep
)
```

#### 5. Update thread_safe_container

```cpp
// thread_safe_container.h - Remove old variant_value references

// Before:
// #include "variant_value.h"
// using ValueVariant = LegacyValueVariant;

// After:
#include "variant_value_v2.h"
// Use variant_value_v2 directly
```

---

## Test Plan

### How will we test this?

#### 1. Build Verification

```bash
# Verify no old symbol references
cmake --build build 2>&1 | grep -E "variant_value\b|LegacyValueVariant"
```

#### 2. Symbol Verification

```bash
# Check built library for old symbols
nm build/lib/libcontainer_system.a | grep variant_value | grep -v v2
# Result: Should be 0
```

#### 3. Functionality Tests

```cpp
TEST(VariantValueRemoval, V2FullyFunctional) {
    // variant_value_v2 provides all functionality
    variant_value_v2 val("test", int32_t{42});

    EXPECT_EQ(val.type(), value_types::int_value);
    EXPECT_EQ(val.name(), "test");

    auto extracted = val.get<int32_t>();
    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, 42);
}

TEST(VariantValueRemoval, TypeIndexCorrect) {
    // All type indices match value_types enum
    variant_value_v2 null_val("n", std::monostate{});
    EXPECT_EQ(static_cast<int>(null_val.type()), 0);

    variant_value_v2 bool_val("b", true);
    EXPECT_EQ(static_cast<int>(bool_val.type()), 1);

    variant_value_v2 string_val("s", std::string{"test"});
    EXPECT_EQ(static_cast<int>(string_val.type()), 13);  // Correct index

    variant_value_v2 bytes_val("bytes", std::vector<uint8_t>{0x01});
    EXPECT_EQ(static_cast<int>(bytes_val.type()), 12);  // Correct index
}
```

#### Success Criteria
- [ ] 0 build errors
- [ ] 0 old symbol references
- [ ] 100% test pass
- [ ] Type index match verified

---

## Benefits

### Advantages After Removal

1. **Data Integrity**: Eliminate data loss risk from type index mismatch
2. **Code Simplification**: Remove duplicate variant implementations
3. **Easier Maintenance**: Only one implementation to manage
4. **Reduced Build Time**: Fewer files to compile

---

## Checklist

- [ ] Search old variant_value usage
- [ ] Update thread_safe_container
- [ ] Remove variant_value.h
- [ ] Remove variant_value.cpp
- [ ] Update CMakeLists.txt
- [ ] Build verification
- [ ] Symbol verification
- [ ] Tests pass

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
