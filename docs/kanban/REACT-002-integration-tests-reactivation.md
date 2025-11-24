# REACT-002: Integration Tests Reactivation

## Metadata
- **ID**: REACT-002
- **Priority**: MEDIUM
- **Estimated Duration**: 1.5 days
- **Dependencies**: REACT-001, REACT-005
- **Status**: DONE
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Migrate the Integration Tests that depended on the legacy value system to the variant-based system to restore system-level testing.

**Current State**:
- Disabled at `CMakeLists.txt:376-383`
- Disabled at `.github/workflows/integration-tests.yml:70`

**Affected Files**:
- `integration_tests/scenarios/container_lifecycle_test.cpp`
- `integration_tests/scenarios/value_operations_test.cpp`
- `integration_tests/performance/serialization_performance_test.cpp`
- `integration_tests/failures/error_handling_test.cpp`

---

## Changes

### How are we changing it?

#### 1. Container Lifecycle Tests Migration (0.5 days)

**Changes**:
- Container creation/deletion tests -> Use `thread_safe_container`
- Value add/get/delete tests -> Use `set_value()` / `get_optimized_value()`

```cpp
// Before
container c;
c.add(std::make_shared<int_value>("id", 123));
auto val = c.get("id");

// After
thread_safe_container c;
c.set_value("id", 123);
auto val = c.get_optimized_value("id");
```

#### 2. Value Operations Tests Migration (0.5 days)

**Changes**:
- Type conversion tests
- Serialization round-trip tests
- Array/Container nesting tests

#### 3. Serialization Performance Tests (0.25 days)

**Changes**:
- Legacy serialization -> variant_value_v2 serialization
- Update performance baseline

#### 4. Error Handling Tests (0.25 days)

**Changes**:
- Update exception handling tests
- Update boundary condition tests

---

## Test Plan

### How will we test it?

#### 1. Local Testing
```bash
cmake -B build -DBUILD_INTEGRATION_TESTS=ON
cmake --build build
cd build && ctest -R integration --output-on-failure
```

#### 2. CI Verification
- Run `.github/workflows/integration-tests.yml` workflow
- Confirm passing on all platforms

#### 3. Performance Baseline Verification
- Verify serialization performance is equal to or better than legacy
- Compare memory usage

#### Success Criteria
- [ ] All integration tests pass
- [ ] messaging_integration functionality works correctly
- [ ] Performance baseline verification complete

---

## Technical Details

### Changes by Test Scenario

| Scenario | Before | After |
|----------|---------|---------|
| Container creation | `container c` | `thread_safe_container c` |
| Value addition | `c.add(shared_ptr)` | `c.set_value(key, value)` |
| Value retrieval | `c.get(key)` | `c.get_optimized_value(key)` |
| Type checking | `dynamic_cast` | `std::get_if` |

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|--------|------|------|-----------|
| Test coverage reduction | Medium | High | 1:1 test case mapping |
| Performance regression | Low | Medium | Benchmark comparison |
| Compatibility issues | Low | High | Use value_bridge |

---

## Checklist

- [x] Migrate container_lifecycle_test.cpp
- [x] Migrate value_operations_test.cpp
- [x] Migrate serialization_performance_test.cpp
- [x] Migrate error_handling_test.cpp
- [x] Enable in CMakeLists.txt
- [ ] Enable integration-tests.yml
- [x] Pass local tests
- [ ] Confirm CI passes

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
