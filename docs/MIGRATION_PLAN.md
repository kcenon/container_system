# Container System: variant_value_v2 Migration Plan

## Executive Summary

This document outlines the plan to migrate the container_system from the legacy polymorphic `value` class hierarchy to the modern type-safe `variant_value_v2` system.

**Timeline:** 3-6 months
**Status:** Phase 1 - Planning & Infrastructure
**Risk Level:** Medium (backward compatibility required)

---

## Goals

### Primary Goals
1. **Type Safety**: Replace runtime polymorphism with compile-time type safety
2. **Performance**: Reduce memory allocations and virtual function overhead
3. **Maintainability**: Simplify codebase by removing 6+ separate value subclasses
4. **Compatibility**: Maintain binary serialization format for existing data

### Non-Goals
- Changing the public API surface (minimize breaking changes)
- Rewriting all existing code at once (incremental migration)
- Supporting pre-C++17 compilers

---

## Current Architecture

### Legacy System (value-based)
```
value (base class, polymorphic)
├── bool_value
├── numeric_value<T> (template)
├── string_value
├── bytes_value
├── container_value
└── array_value
```

**Characteristics:**
- Virtual function dispatch for all operations
- Heap allocation per value (std::shared_ptr everywhere)
- 16 different value_types (enum)
- Separate .h/.cpp for each type

### Modern System (variant_value_v2)
```
variant_value_v2 (single class)
└── ValueVariant (std::variant)
    ├── std::monostate (null)
    ├── bool
    ├── int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
    ├── float, double
    ├── std::vector<uint8_t> (bytes)
    ├── std::string
    ├── std::shared_ptr<thread_safe_container>
    └── array_variant (recursive)
```

**Characteristics:**
- Zero virtual function overhead
- Type index matches value_types enum exactly
- Platform-aware (handles macOS vs Windows long long differences)
- Thread-safe with shared_mutex
- Perfect serialization round-trip

---

## Migration Phases

### Phase 1: Infrastructure (Current - Week 4)
**Status:** ✅ Completed

- [x] Implement variant_value_v2 core
- [x] Create value_bridge for bidirectional conversion
- [x] Add compatibility tests
- [x] Create migration documentation
- [x] Set up feature branch

**Deliverables:**
- `internal/variant_value_v2.h/.cpp`
- `integration/value_bridge.h/.cpp`
- `tests/variant_value_v2_compatibility_tests.cpp`
- This migration plan

---

### Phase 2: Core Container Migration (Week 5-8)
**Status:** 🔄 In Progress

**Tasks:**
1. Add variant_value_v2 support to `value_container`
   - [ ] Add overloaded constructors accepting `variant_value_v2`
   - [ ] Add `set_units(std::vector<variant_value_v2>)`
   - [ ] Maintain backward compatibility with `std::shared_ptr<value>`
   - [ ] Update internal storage to support both types

2. Create factory functions for variant_value_v2
   - [ ] `make_bool_value(name, value)`
   - [ ] `make_numeric_value<T>(name, value)`
   - [ ] `make_string_value(name, value)`
   - [ ] `make_bytes_value(name, data)`
   - [ ] `make_array_value(name, values)`
   - [ ] `make_container_value(name, container)`

3. Update `thread_safe_container` integration
   - [ ] Ensure variant_value_v2 can store containers
   - [ ] Test recursive container serialization

**Acceptance Criteria:**
- All existing tests pass
- New tests for variant_value_v2 paths pass
- No performance regression
- Serialization format unchanged

---

### Phase 3: Test Migration (Week 9-12)
**Status:** ⏳ Pending

**Tasks:**
1. Update unit tests
   - [ ] `tests/value_tests.cpp` → use variant_value_v2
   - [ ] `tests/container_tests.cpp` → dual mode tests
   - [ ] `tests/serialization_tests.cpp` → verify compatibility

2. Update integration tests
   - [ ] `integration_tests/scenarios/value_operations_test.cpp`
   - [ ] `integration/messaging_integration.*` → support both modes

3. Update benchmarks
   - [ ] `benchmarks/value_operations_bench.cpp`
   - [ ] Add variant_value_v2 vs legacy comparison
   - [ ] Document performance improvements

**Acceptance Criteria:**
- 100% test pass rate with variant_value_v2
- Benchmarks show >= 10% performance improvement
- Memory usage reduced by >= 20%

---

### Phase 4: Examples & Documentation (Week 13-16)
**Status:** ⏳ Pending

**Tasks:**
1. Update examples
   - [ ] `examples/` directory → use variant_value_v2
   - [ ] Add migration examples
   - [ ] Update README samples

2. Documentation updates
   - [ ] API reference for variant_value_v2
   - [ ] Migration guide for users
   - [ ] Performance comparison charts
   - [ ] Architecture decision records (ADR)

3. Deprecation warnings
   - [ ] Add `[[deprecated]]` to legacy value classes
   - [ ] Compiler warnings guide users to new API
   - [ ] Migration script/tool

**Acceptance Criteria:**
- All examples compile and run
- Documentation is complete and accurate
- Clear migration path for existing users

---

### Phase 5: Legacy Cleanup (Week 17-24)
**Status:** ⏳ Pending

**Tasks:**
1. Soft deprecation (Week 17-20)
   - [ ] Mark legacy classes as deprecated
   - [ ] Monitor usage via deprecation warnings
   - [ ] Provide migration support

2. Hard deprecation (Week 21-24)
   - [ ] Remove `values/` directory (bool_value, numeric_value, etc.)
   - [ ] Simplify value_bridge (one-way legacy → modern only)
   - [ ] Remove legacy serialization code paths

3. Final cleanup
   - [ ] Remove unused code
   - [ ] Update build system (remove legacy .cpp files)
   - [ ] Final performance validation

**Acceptance Criteria:**
- No legacy value classes remain
- Codebase size reduced by >= 30%
- All tests pass
- No regressions in functionality

---

## Technical Details

### Conversion Strategy

#### Legacy → Modern (via value_bridge)
```cpp
// Old code
auto legacy_val = std::make_shared<string_value>("key", "hello");

// New code (direct)
auto modern_val = variant_value_v2("key", std::string("hello"));

// Migration path (automatic)
auto modern_val = value_bridge::to_modern(legacy_val);
```

#### Type Mapping
| Legacy Class | value_types | variant_value_v2 Type | Index |
|--------------|-------------|----------------------|-------|
| null | null_value (0) | std::monostate | 0 |
| bool_value | bool_value (1) | bool | 1 |
| numeric_value<short> | short_value (2) | int16_t | 2 |
| numeric_value<int> | int_value (4) | int32_t | 4 |
| string_value | string_value (13) | std::string | 13 |
| bytes_value | bytes_value (12) | std::vector<uint8_t> | 12 |
| array_value | array_value (15) | array_variant | 15 |
| container_value | container_value (14) | std::shared_ptr<thread_safe_container> | 14 |

### Serialization Compatibility

**Wire Format (unchanged):**
```
[name_len:4][name:UTF-8][type:1][data:variable]
```

- Type byte matches value_types enum (0-15)
- Both systems produce identical binary output
- Deserialization auto-detects and converts

### Performance Targets

| Metric | Legacy | Target | Measured |
|--------|--------|--------|----------|
| Memory per value | ~80 bytes | ~48 bytes | TBD |
| Construction time | 100% | 60% | TBD |
| Copy time | 100% | 70% | TBD |
| Type conversion | 100% | 40% | TBD |

---

## Risk Assessment

### High Risk
1. **Serialization compatibility**
   - Mitigation: Extensive round-trip testing
   - Status: ✅ Tests in place

2. **Breaking API changes**
   - Mitigation: Maintain dual support during transition
   - Status: 🔄 value_bridge provides compatibility layer

### Medium Risk
1. **Performance regressions**
   - Mitigation: Continuous benchmarking
   - Status: ⏳ Benchmarks to be added in Phase 3

2. **Thread safety issues**
   - Mitigation: Thread sanitizer testing
   - Status: ✅ variant_value_v2 uses shared_mutex

### Low Risk
1. **Compilation issues**
   - Mitigation: CI/CD on multiple platforms
   - Status: ⏳ To be added

---

## Testing Strategy

### Unit Tests
- [x] variant_value_v2 core functionality
- [ ] Factory functions
- [ ] Thread safety tests
- [ ] Platform-specific tests (macOS, Linux, Windows)

### Integration Tests
- [x] value_bridge conversions
- [ ] container integration
- [ ] Messaging system integration
- [ ] Serialization round-trips

### Performance Tests
- [ ] Memory usage benchmarks
- [ ] CPU performance benchmarks
- [ ] Comparison with legacy system

### Compatibility Tests
- [x] Legacy → Modern conversion
- [x] Modern → Legacy conversion
- [x] Round-trip verification
- [ ] Binary format compatibility

---

## Success Metrics

### Quantitative
- ✅ Zero test failures
- ⏳ >= 10% performance improvement
- ⏳ >= 20% memory reduction
- ⏳ >= 30% code size reduction
- ⏳ 100% serialization compatibility

### Qualitative
- ✅ Simpler mental model (single class vs class hierarchy)
- ⏳ Easier to maintain (fewer files)
- ⏳ Better compile-time safety
- ⏳ Clearer error messages

---

## Rollback Plan

If critical issues arise:

1. **Phase 1-2**: Easy rollback (feature branch)
2. **Phase 3-4**: Revert commits, keep legacy system
3. **Phase 5**: Point of no return (should be well-tested by then)

**Rollback triggers:**
- > 5% performance regression
- Unsolvable serialization compatibility issues
- Critical production bugs

---

## Communication Plan

### Internal Team
- Weekly status updates in team meeting
- Migration progress dashboard
- Code review guidelines for variant_value_v2

### External Users (if applicable)
- Migration guide published
- Deprecation timeline announced
- Support for migration questions

---

## Next Steps (Immediate)

1. ✅ Create this migration plan
2. 🔄 Implement factory functions for variant_value_v2
3. ⏳ Add variant_value_v2 support to value_container
4. ⏳ Write comprehensive unit tests
5. ⏳ Set up performance benchmarking

---

## Appendix

### Related Documents
- [VALUE_SYSTEM_COMPARISON_ANALYSIS.md](VALUE_SYSTEM_COMPARISON_ANALYSIS.md)
- [VARIANT_VALUE_V2_MIGRATION_GUIDE.md](VARIANT_VALUE_V2_MIGRATION_GUIDE.md)

### Key Files
- `internal/variant_value_v2.h` - Modern implementation
- `integration/value_bridge.h` - Conversion layer
- `core/value.h` - Legacy base class
- `core/value_types.h` - Type enumeration (shared)

---

*Last Updated: 2025-11-06*
*Version: 1.0*
*Status: Phase 2 - Core Container Migration*
