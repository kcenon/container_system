---
doc_id: "CNT-ADR-003"
doc_title: "ADR-003: SIMD Optimization Strategy"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Accepted"
project: "container_system"
category: "ADR"
---

# ADR-003: SIMD Optimization Strategy

> **SSOT**: This document is the single source of truth for **ADR-003: SIMD Optimization Strategy**.

| Field | Value |
|-------|-------|
| Status | Accepted |
| Date | 2025-06-01 |
| Decision Makers | kcenon ecosystem maintainers |

## Context

container_system provides value containers and serialization primitives used across
the kcenon ecosystem. Batch operations on value arrays (parsing, comparison, aggregation)
are performance-critical in downstream projects, particularly monitoring_system
(metric aggregation) and pacs_system (DICOM data processing).

SIMD (Single Instruction, Multiple Data) can accelerate these batch operations by
processing 4-8 values per CPU instruction. However, SIMD introduces platform-specific
code and requires careful abstraction to maintain portability.

Target platforms:
- x86_64: SSE4.2, AVX2 (Intel/AMD desktop and server)
- ARM64: NEON (Apple Silicon, ARM servers, Raspberry Pi)
- Fallback: Scalar code for unsupported platforms

## Decision

**Implement SIMD-accelerated batch operations** in `simd_batch.h` with compile-time
platform detection and runtime feature checks.

Design:
1. **`simd_batch<T>`** — Template class providing batch operations (sum, min, max,
   find, comparison) on contiguous arrays of arithmetic types.
2. **Compile-time dispatch** — Preprocessor macros (`__AVX2__`, `__ARM_NEON`)
   select the optimal instruction set at build time.
3. **Scalar fallback** — Every SIMD operation has a portable scalar implementation
   as the default code path.
4. **No external SIMD libraries** — Direct intrinsics (`_mm256_*`, `vld1q_*`) to
   avoid additional dependencies.

```cpp
// Batch sum using SIMD-accelerated path
simd_batch<float> batch(data.data(), data.size());
float total = batch.sum();  // AVX2 on x86_64, NEON on ARM64, scalar fallback
```

## Alternatives Considered

### External SIMD Library (Highway, xsimd, Vc)

- **Pros**: Mature, portable SIMD abstraction; handles runtime dispatch.
- **Cons**: Additional dependency for the Tier 1 foundation library. Highway
  and xsimd have non-trivial build requirements. Increases package management
  burden across 8 repositories.

### Compiler Auto-Vectorization Only

- **Pros**: No platform-specific code, relies on `-O2`/`-O3` optimization.
- **Cons**: Unreliable — vectorization depends on compiler version, loop structure,
  and aliasing analysis. Performance varies significantly across compilers and is
  difficult to guarantee or benchmark reproducibly.

### Runtime CPU Feature Detection (CPUID)

- **Pros**: Single binary supports all instruction sets, selects optimal at runtime.
- **Cons**: Adds runtime overhead for feature detection, complicates the hot path
  with function pointer dispatch. Unnecessary for the ecosystem's CI matrix which
  builds per-platform.

## Consequences

### Positive

- **Measurable speedup**: 3-5x for batch operations on aligned data vs. scalar,
  verified in benchmarks.
- **Zero runtime dispatch overhead**: Compile-time selection means no function
  pointer indirection in the hot path.
- **Portable**: Scalar fallback ensures correct behavior on all platforms.
- **No new dependencies**: Direct intrinsics keep container_system's dependency
  footprint minimal.

### Negative

- **Platform-specific code**: SIMD intrinsics must be maintained for each target
  architecture (currently x86_64 AVX2 and ARM64 NEON).
- **Alignment requirements**: SIMD operations require aligned memory. Misaligned
  data falls back to scalar or uses unaligned loads with reduced performance.
- **Limited type support**: SIMD batch operations currently support `float`,
  `double`, and integer types. Complex types require scalar processing.
- **Testing burden**: Each platform's SIMD path must be independently tested in CI.
