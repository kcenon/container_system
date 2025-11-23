# REACT-001: Unit Tests Reactivation

## Metadata
- **ID**: REACT-001
- **Priority**: HIGH
- **Estimated Duration**: 1.5 days
- **Dependencies**: None
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Migrate and reactivate the Unit Tests that were disabled when the legacy polymorphic `value` class system was removed, updating them to use the new variant-based API.

**Current State**:
- Disabled at `CMakeLists.txt:376-383`
- Using legacy `value` class and `add()` method

**Affected Files**:
- `tests/unit_tests.cpp`
- `tests/benchmark_tests.cpp`
- `tests/performance_tests.cpp`
- `tests/test_messaging_integration.cpp`
- `tests/variant_value_v2_compatibility_tests.cpp`

---

## Changes

### How are we changing it?

#### 1. Update Test Helpers (0.5 days)

```cpp
// Before: Legacy API
auto val = std::make_shared<int_value>("count", 42);
container.add(val);

// After: Variant-based API
optimized_value val{"count", value_types::int_value, 42};
container.set_optimized_value(val);
// or
container.set_value("count", 42);
```

**Work Items**:
- [ ] Create `test_helpers_v2.h` - variant-based utility functions
- [ ] Implement `make_optimized_value()` helper function
- [ ] Update `system_fixture.h`

#### 2. Unit Tests Migration (1 day)

**Migration Patterns**:

| Legacy Pattern | Modern Pattern |
|------------|-------------|
| `std::make_shared<int_value>()` | `optimized_value{}` |
| `container.add(val)` | `container.set_value()` |
| `dynamic_cast<int_value*>()` | `std::get_if<int>()` |
| `val->to_int()` | `*std::get_if<int>(&val->data)` |

**Test Files to Migrate**:
- [ ] `unit_tests.cpp`: Core container operations
- [ ] `performance_tests.cpp`: Basic performance validation
- [ ] `test_long_range_checking.cpp`: Type range tests

#### 3. Update CMakeLists.txt

```cmake
# Needs to be uncommented
if(USE_UNIT_TEST)
    add_subdirectory(tests)
endif()
```

---

## Test Plan

### How will we test it?

#### 1. Build Verification
```bash
cmake -B build -DUSE_UNIT_TEST=ON
cmake --build build
```

#### 2. Test Execution
```bash
cd build && ctest --output-on-failure
```

#### 3. CI Verification
- Verify test workflow runs on GitHub Actions
- Confirm passing on all platforms: Ubuntu GCC, Clang, macOS

#### Success Criteria
- [ ] All core unit tests pass
- [ ] CI integration tests activated
- [ ] Code coverage restored to 80% or above

---

## Technical Details

### Migration Patterns

#### Pattern 1: Value Creation
```cpp
// Before
auto val = std::make_shared<int_value>("count", 42);

// After
optimized_value val{"count", value_types::int_value, 42};
```

#### Pattern 2: Type Checking
```cpp
// Before
auto val = container.get_value("count");
if (auto* int_val = dynamic_cast<int_value*>(val.get())) {
    int v = int_val->get();
}

// After
if (auto val = container.get_optimized_value("count")) {
    if (auto* v = std::get_if<int>(&val->data)) {
        int value = *v;
    }
}
```

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|--------|------|------|-----------|
| Missing tests | Low | Medium | Cross-reference with existing test list |
| API mismatch | Medium | Low | Early detection via compile errors |
| Performance degradation | Low | Low | Benchmark comparison |

---

## Checklist

- [ ] Create `test_helpers_v2.h`
- [ ] Implement `make_optimized_value()`
- [ ] Update `system_fixture.h`
- [ ] Migrate `unit_tests.cpp`
- [ ] Migrate `performance_tests.cpp`
- [ ] Migrate `test_long_range_checking.cpp`
- [ ] Enable tests in CMakeLists.txt
- [ ] Pass local ctest
- [ ] Confirm CI passes

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
