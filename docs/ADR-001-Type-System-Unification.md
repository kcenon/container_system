# ADR-001: Type System Unification

**Date**: 2025-11-08
**Status**: Accepted
**Deciders**: Lead Architect, Senior Developers

---

## Context

The container_system currently maintains **three competing type systems** that coexist within the same codebase:

1. **Polymorphic Value System** (Legacy)
   - Virtual dispatch-based polymorphism
   - Base class: `value`
   - Derived classes: `int_value`, `string_value`, etc.
   - Runtime overhead due to virtual function calls
   - Heap allocations for all values

2. **variant_value (v1)** - **INCORRECT Type Ordering**
   - Uses `std::variant` for type-safe storage
   - **Problem**: Type index does NOT match enum values
   - Causes serialization/deserialization incompatibility
   - Example: `bytes` is index 2 in variant but should be different in enum

3. **variant_value_v2** - **CORRECT Implementation**
   - Uses `std::variant` with correct type ordering
   - Type index matches enum exactly
   - Compile-time type safety
   - Better performance (no virtual dispatch)
   - Supports `std::visit` for type-safe operations

### Current State Problems

- **3x Maintenance Burden**: Every type operation must be implemented three times
- **Conversion Overhead**: `value_bridge.h` required for cross-system conversion
- **Developer Confusion**: New contributors don't know which system to use
- **Incomplete Migration**: No clear direction or timeline
- **Serialization Issues**: Different systems produce incompatible serialized formats

---

## Decision

**We will adopt `variant_value_v2` as the primary and only type system for container_system.**

### Migration Timeline

#### Phase 1: Deprecation (Current Version - v2.x)
- Mark `value` (polymorphic) with `[[deprecated]]` attribute
- Mark `variant_value` (v1) with `[[deprecated]]` attribute
- Add compiler warnings with migration guidance
- Document migration path in `docs/MIGRATION.md`

#### Phase 2: Coexistence Period (6 months minimum)
- All three systems remain functional
- New features only added to `variant_value_v2`
- Provide migration tools in `tools/migrate_to_v2.h`
- Update all examples to use `variant_value_v2`

#### Phase 3: Next Major Version (v3.0.0)
- **Remove** polymorphic value system entirely
- **Remove** variant_value v1 entirely
- **Remove** value_bridge.h
- **Rename** `variant_value_v2` → `value` (becomes the new `value` type)

---

## Rationale

### Why variant_value_v2?

1. **Type Safety**
   - Compile-time type checking via `std::variant`
   - Eliminates runtime type cast failures
   - Compiler enforces exhaustive pattern matching

2. **Performance**
   - No virtual function overhead
   - Small object optimization (SOO) for common types
   - Zero-cost abstractions

3. **Modern C++20 Features**
   - Leverages `std::visit` for type-safe dispatch
   - Supports structured bindings
   - Compatible with constexpr operations

4. **Correct Semantics**
   - Type index matches enum exactly (unlike v1)
   - Serialization format is deterministic
   - Cross-version compatibility guaranteed

5. **Standard Library Integration**
   - Works naturally with STL algorithms
   - Compatible with ranges and views
   - Better error messages from compilers

### Why NOT Keep Multiple Systems?

1. **Maintenance Burden**
   - Every bug fix must be applied three times
   - Testing complexity increases exponentially
   - Documentation becomes outdated quickly

2. **Inconsistent Behavior**
   - Same operation may succeed in one system, fail in another
   - Performance characteristics differ wildly
   - Debugging is confusing when crossing system boundaries

3. **Blocks Innovation**
   - New features delayed due to multi-system support
   - Cannot adopt modern C++ features in legacy code
   - Technical debt accumulates faster than it's paid down

---

## Consequences

### Positive

- **Single Source of Truth**: One type system to learn, document, and maintain
- **Performance Improvement**: Eliminate virtual dispatch and heap allocations
- **Modern Codebase**: Full adoption of C++20 features
- **Clear Direction**: No ambiguity about which system to use
- **Reduced Binary Size**: Remove dead code from polymorphic system

### Negative

- **Breaking Change**: Requires all users to migrate by v3.0.0
- **Migration Effort**: Users must update their code
- **Short-term Maintenance**: Must support deprecated systems during transition
- **Documentation Update**: All tutorials and guides need revision

### Risks and Mitigation

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Users resist migration | High | Medium | Provide automated migration tools, clear documentation, 6-month minimum notice |
| Serialization compatibility breaks | High | Low | Implement bidirectional converters, extensive compatibility tests |
| Performance regression in specific cases | Medium | Low | Comprehensive benchmarks, optimization where needed |
| Migration tools have bugs | Medium | Medium | Extensive testing, phased rollout, quick-fix releases |

---

## Migration Path

### For Library Maintainers

1. **Sprint 1-2** (Current)
   - Write this ADR
   - Add deprecation warnings
   - Create migration guide

2. **Sprint 3-4** (Weeks 5-8)
   - Implement `value_migrator` utility class
   - Add roundtrip serialization tests
   - Update all internal code to use `variant_value_v2`

3. **Sprint 5-6** (Weeks 9-12)
   - Update documentation and examples
   - Create migration automation script
   - Announce deprecation timeline to users

4. **Next Major Release** (6+ months)
   - Remove deprecated code
   - Rename `variant_value_v2` → `value`
   - Release v3.0.0

### For Library Users

```cpp
// Before (using polymorphic value)
auto value = std::make_shared<int_value>(42);
container->add("key", value);

// After (using variant_value_v2)
auto value = variant_value_v2(42);
container->add("key", value);

// Or use migration tool
#include <container/tools/migrate_to_v2.h>
auto v2 = value_migrator::from_polymorphic(*old_value);
```

---

## Alternatives Considered

### Alternative 1: Keep All Three Systems
**Rejected** - Unsustainable maintenance burden, continues confusion, blocks progress.

### Alternative 2: Fix variant_value v1 Type Ordering
**Rejected** - Would break existing serialization, essentially creates a "v1.5" system, doesn't reduce system count.

### Alternative 3: Create variant_value v3 with Even Better Features
**Rejected** - Violates "Don't Let Perfect Be the Enemy of Good". variant_value_v2 is sufficient and already implemented.

### Alternative 4: Gradual System Merge
**Rejected** - Prolonged transition period, unclear migration path, technical debt persists longer.

---

## References

- [IMPROVEMENT_PLAN.md](../IMPROVEMENT_PLAN.md) - Full improvement roadmap
- [SYSTEM_ANALYSIS_SUMMARY.md](../../SYSTEM_ANALYSIS_SUMMARY.md) - System-wide analysis
- [C++ Core Guidelines - Variants](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-var) - Best practices for std::variant usage

---

## Decision Log

- **2025-11-08**: ADR created and accepted
- **Next Review**: 2025-12-08 (1 month) - Review migration progress

---

## Signatures

**Lead Architect**: [Accepted]
**Senior Developer (Container Team)**: [Accepted]
**Senior Developer (Integration Team)**: [Accepted]
