# container_system

## Overview

High-performance, type-safe, serializable data container library for messaging systems
and general-purpose applications. Provides SIMD-accelerated operations, multiple
serialization formats, thread-safe concurrent access, and a builder pattern for
message construction.

## Architecture

```
include/kcenon/container/
  core/           - value_types (16 types), value_store, container_schema
  core/serializers/ - binary, json, xml, msgpack serializers + serializer_factory
  internal/       - value, variant_value_factory, memory_pool, pool_allocator,
                    thread_safe_container, simd_processor, rcu_value, epoch_manager
  internal/async/ - C++20 coroutine support (task, generator, async_container)
  messaging/      - message_container (domain-specific)
  integration/    - messaging_integration
  utilities/      - String conversion, core utilities
```

Key abstractions:
- `value_container` / `message_buffer` — Primary serializable container (same class, `message_buffer` preferred since v2.0.0)
- `value_types` enum — 16 types (null, bool, short/ushort, int/uint, long/ulong, llong/ullong, float, double, string, bytes, container, array)
- `serializer_strategy` — Abstract interface (Strategy pattern) with 4 implementations
- `thread_safe_container` — Synchronized write wrapper with RCU-based lock-free reads
- `simd_processor` — SIMD acceleration (ARM NEON, x86 AVX2 auto-detected)
- `memory_pool` / `pool_allocator` — Custom memory allocation for small objects

## Build & Test

```bash
# Build script (recommended)
./scripts/build.sh

# CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
cd build && ctest --output-on-failure
```

Key CMake options:
- `BUILD_WITH_COMMON_SYSTEM` (ON) — Mandatory common_system integration
- `BUILD_TESTS` (ON) — Unit tests (Google Test 1.17.0)
- `ENABLE_MESSAGING_FEATURES` (ON) — Messaging optimizations
- `CONTAINER_BUILD_BENCHMARKS` (OFF) — Benchmarks
- `CONTAINER_USE_MEMORY_POOL` (ON) — Memory pool for small allocations
- `CONTAINER_ENABLE_COROUTINES` (ON) — C++20 coroutine async API

Presets: `default`, `debug`, `release`, `core-debug`, `core-release`, `asan`, `tsan`, `ubsan`, `ci`, `vcpkg`

CI: Multi-platform (Ubuntu GCC/Clang, macOS, Windows MSVC), coverage, static analysis,
sanitizers, benchmarks, fuzzing, CVE scan, SBOM.

## Key Patterns

- **Type-safe containers** — 16-type enum with compile-time `constexpr` type mapping
- **SIMD optimization** — Auto-detected ARM NEON / x86 AVX2; ~3.2x speedup on Apple Silicon
- **Serializer strategy** — `serializer_factory` resolves binary/json/xml/msgpack at runtime
- **Thread safety** — `thread_safe_container` for writes, RCU (`rcu_value`, `epoch_manager`) for lock-free reads
- **Memory pool** — Custom `pool_allocator` for small allocation performance
- **Coroutine async** — `task<T>`, `generator<T>`, `async_container` via C++20 coroutines
- **Builder pattern** — `messaging_container_builder` for message construction

## Ecosystem Position

**Tier 1** — Data serialization layer between foundation and application systems.

```
common_system (Tier 0) [required]
container_system (Tier 1) <-- this project
  -> used by: network_system, database_system, pacs_system
```

Used as the data exchange format across the entire ecosystem.

## Dependencies

**Required**: kcenon-common-system, C++20, CMake 3.20+
**Dev/test**: Google Test 1.17.0, Google Benchmark 1.9.5
**Optional**: gRPC 1.60.0 + protobuf 4.25.1 (isolated gRPC module only)

## Known Constraints

- common_system is mandatory (`BUILD_WITH_COMMON_SYSTEM=OFF` triggers `FATAL_ERROR`)
- C++20 required (GCC 11+, Clang 14+, Apple Clang 14+, MSVC 2022+)
- C++20 modules experimental (CMake 3.28+, Clang 16+/GCC 14+)
- gRPC module isolated: separate build via `cmake -S grpc -B build-grpc`
- Legacy API disabled by default since v0.2.0
- SIMD performance varies by platform (NEON on ARM64, AVX2 on x86_64)
- UWP/Xbox unsupported
