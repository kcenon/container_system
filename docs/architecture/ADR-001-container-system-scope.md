# ADR-001: Container System Scope and Purpose

**Status**: Accepted
**Date**: 2026-01-31
**Authors**: @kcenon
**Related Issues**: #345, #346

## Context

The container_system repository currently contains a full copy of common_system (~50 files) with only one unique header file (`include/container/optimizations/fast_parser.h`). This raises questions about the purpose and scope of container_system:

1. **Unclear boundaries**: What should belong in container_system vs. common_system?
2. **Duplication issues**: Full copy of common_system causes version drift and maintenance burden
3. **Scope ambiguity**: Is container_system for data containers, serialization, or something else?

## Decision

### container_system Purpose

**Primary Purpose**: High-performance serializable data container library

container_system provides:
- Type-safe heterogeneous key-value storage (`value_container`)
- Multiple serialization formats (Binary, JSON, XML)
- SIMD-accelerated operations for numeric arrays
- Thread-safe concurrent access patterns
- Messaging system integration helpers

### Scope Boundaries

#### In Scope (belongs in container_system)

| Category | Examples | Rationale |
|----------|----------|-----------|
| **Core container** | `value_container`, `message_buffer`, `variant_value` | Primary data structure |
| **Value types** | `value_types.h`, type-safe value creation | Type system for container |
| **Serialization** | Binary, JSON, XML serializers, format detection | Data interchange formats |
| **Performance** | SIMD processors, batch operations, memory pools | Performance optimizations specific to containers |
| **Thread safety** | `thread_safe_container`, read-write locks | Concurrent access patterns |
| **Integration** | Messaging builders, network serializers, database adapters | Domain-specific helpers |
| **Optimizations** | `fast_parser.h`, storage policies | Container-specific optimizations |

#### Out of Scope (belongs in common_system)

| Category | Examples | Rationale |
|----------|----------|-----------|
| **General utilities** | Logging, configuration, string utilities | Cross-cutting concerns |
| **DI infrastructure** | Dependency injection, service locator | Application-wide patterns |
| **Error handling** | `Result<T>`, error codes, exceptions | Foundation-level concerns |
| **Interfaces** | Abstract base classes, concepts | Shared abstractions |
| **Bootstrap** | Application startup, initialization | Framework concerns |

### Dependency Strategy

Replace the full common_system copy with proper CMake dependency management:

```cmake
include(FetchContent)

FetchContent_Declare(
    common_system
    GIT_REPOSITORY https://github.com/kcenon/common_system.git
    GIT_TAG main  # Or specific version tag
)

FetchContent_MakeAvailable(common_system)

target_link_libraries(container_system PUBLIC common_system::common)
```

## Consequences

### Positive

1. **Clear boundaries**: Developers know where to add new features
2. **Reduced duplication**: No more version drift or duplicate bug fixes
3. **Smaller repository**: Only ~5-10 unique source files instead of 50+ copied files
4. **Better separation of concerns**: Container logic separate from general utilities
5. **Easier testing**: Clearer test boundaries and dependencies

### Negative

1. **Network dependency**: Builds require fetching common_system (can be cached)
2. **Build time**: Initial fetch adds ~5-10 seconds (one-time cost)
3. **Version coupling**: Need to track compatible common_system versions

### Mitigation

- Use CMake `FetchContent` caching to minimize network overhead
- Pin to specific common_system version tags for stability
- Document offline build procedures for airgapped environments
- Provide pre-built packages that bundle dependencies

## Components Breakdown

### Unique container_system Components

| Component | Purpose | Status |
|-----------|---------|--------|
| `core/container.h` | Main container implementation | Core ✅ |
| `core/value_types.h` | Container-specific type system | Core ✅ |
| `internal/simd_processor.h` | SIMD acceleration for containers | Optimization ✅ |
| `integration/messaging_integration.h` | Messaging system helpers | Integration ✅ |
| `include/container/optimizations/fast_parser.h` | Parser optimization utilities | Optimization ✅ |

### Shared Dependencies (from common_system)

| Component | Purpose | Source |
|-----------|---------|--------|
| `Result<T>` | Error handling | common_system |
| Logging infrastructure | Diagnostics | common_system |
| DI container | Dependency injection | common_system |
| Error codes | Standardized errors | common_system |
| Utilities | String, file, etc. | common_system |

## Implementation Plan

See [Epic #345](https://github.com/kcenon/container_system/issues/345) for detailed implementation tasks:

1. **Phase 1**: Document scope (this ADR) ✅
2. **Phase 2**: Add CMake FetchContent for common_system
3. **Phase 3**: Update includes to reference external common_system
4. **Phase 4**: Remove local common_system/ directory
5. **Phase 5**: Verify all tests pass with external dependency

## Alternatives Considered

### Alternative 1: Keep full common_system copy

**Rejected**: Violates DRY principle, causes version drift, maintenance burden

### Alternative 2: Merge container_system into common_system

**Rejected**: Container system is substantial enough to warrant separate repository:
- ~15K lines of container-specific code
- High-performance optimizations (SIMD)
- Domain-specific features (serialization, messaging)

### Alternative 3: Create minimal common subset

**Rejected**: Creates third repository to maintain, unclear boundaries

## References

- [Kent Beck's Four Rules of Simple Design](http://wiki.c2.com/?XpSimplicityRules)
- [Epic #345: Replace common_system copy](https://github.com/kcenon/container_system/issues/345)
- [Issue #346: Document container_system scope](https://github.com/kcenon/container_system/issues/346)
- [common_system Repository](https://github.com/kcenon/common_system)

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-31 | @kcenon | Initial ADR documenting scope decisions |
