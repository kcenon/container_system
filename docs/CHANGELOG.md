# Changelog

All notable changes to the Container System project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Language:** **English** | [한국어](CHANGELOG_KO.md)

---

## [Unreleased]

### Added
- **Async Container Operations** (#266): Add coroutine-based async API for container serialization (Phase 2)
  - Add `async_container` class wrapping `value_container` for async operations
  - Add `serialize_async()` method for non-blocking byte array serialization
  - Add `serialize_string_async()` method for non-blocking string serialization
  - Add `deserialize_async()` static method for async deserialization from bytes
  - Add `deserialize_string_async()` static method for async deserialization from string
  - Add thread-pool integration via `detail::async_awaitable` for CPU-bound work offloading
  - Add convenience methods: `set()`, `get()`, `contains()` forwarding to underlying container
  - Add 7 comprehensive unit tests for async container operations
  - Support both Result-based and exception-based error handling APIs
  - Require C++20 coroutine support

- **MessagePack Serialization Support** (#234): Add MessagePack binary format as alternative to JSON/XML
  - Add `msgpack_encoder` class for compact binary serialization
  - Add `msgpack_decoder` class for binary deserialization
  - Add `to_msgpack()` method for serializing containers to MessagePack format
  - Add `from_msgpack()` method for deserializing from MessagePack data
  - Add `create_from_msgpack()` static factory method
  - Add `to_msgpack_result()` and `from_msgpack_result()` Result-based APIs
  - Add `serialization_format` enum for format identification
  - Add `detect_format()` method for automatic format detection
  - Add 44 comprehensive unit tests for encoder, decoder, and integration
  - MessagePack provides 50-80% smaller output than JSON with 2-4x faster parsing
  - Full support for all container value types including nested containers

- **Result-Returning Core API Tests** (#238): Add comprehensive unit tests for Result-returning value APIs
  - Add 15 test cases covering set_result(), set_all_result(), and remove_result() methods
  - Test success paths for all Result-returning methods with various data types
  - Test empty key error handling with error_codes::empty_key verification
  - Test key not found error handling with error_codes::key_not_found verification
  - Test edge cases: special characters, long keys, unicode keys in key names
  - Test bulk operations with set_all_result() including partial failure scenarios
  - Verify error messages are descriptive and contain relevant context

- **C++20 Coroutine Async API** (#233): Add coroutine-based async types for non-blocking operations (Phase 1)
  - Add `task<T>` coroutine type for async operations with proper exception propagation
  - Add `task<void>` specialization for void-returning coroutines
  - Add `generator<T>` for lazy sequence generation with STL-compatible iteration
  - Add `make_ready_task()` and `make_exceptional_task()` helper functions
  - Add `take()` helper for limiting generator output
  - Add CMake option `CONTAINER_ENABLE_COROUTINES` (default: ON)
  - Add 19 comprehensive unit tests for coroutine types
  - Support symmetric transfer for efficient coroutine chaining
  - Require C++20 with `<coroutine>` header support (GCC 10+, Clang 13+, MSVC 2019 16.8+)

- **True Lock-Free RCU Reader** (#232): Implement lock-free read path using Read-Copy-Update pattern
  - Add `rcu_value<T>` template class for wait-free atomic reads with copy-on-write updates
  - Add `lockfree_container_reader` class with true lock-free reads (no mutex acquisition)
  - Add `epoch_manager` singleton for epoch-based memory reclamation
  - Add `epoch_guard` RAII helper for automatic critical section management
  - Add `create_lockfree_reader()` factory method to `thread_safe_container`
  - Provide wait-free O(1) reads with snapshot-based isolation
  - Support compare-and-swap updates via `compare_and_update()` method
  - Add 12 comprehensive unit tests for lock-free functionality
  - Expected 5-50x performance improvement for read-heavy concurrent workloads

- **Error Codes Unit Tests** (#237): Add comprehensive unit tests for error codes module
  - Add 34 test cases covering all error codes and utility functions
  - Test error code value verification for all categories (1xx-5xx)
  - Test get_message() returns correct messages for all error codes
  - Test get_category() returns correct category names
  - Test is_*_error() helper functions for category classification
  - Test make_message() with various inputs including edge cases
  - Test constexpr evaluation of utility functions
  - Test boundary values between categories
  - Add Error Codes section to API_REFERENCE.md documentation

- **Schema Constraint Validators Test Coverage** (#246): Add comprehensive unit tests for schema validation constraints
  - Add 43 test cases covering all constraint validators
  - Add tests for integer range validation (boundary, negative, long long types)
  - Add tests for floating-point range validation (double and float types)
  - Add tests for string/bytes length validation
  - Add tests for regex pattern matching (email, phone, UUID patterns)
  - Add tests for one_of enum-style validation (case-sensitive)
  - Add tests for custom validator functions
  - Add tests for nested schema validation
  - Add tests for combined constraints and edge cases
  - Add tests for schema utility methods (field_count, has_field, is_required)
  - Add tests for schema copy/move semantics

### Fixed
- **Schema range() overload ambiguity** (#250): Fix Linux/GCC build failure caused by ambiguous range() overloads
  - Use C++20 concepts (std::integral and std::floating_point) to disambiguate between integer and floating-point range constraints
  - Replace non-template range() overloads with template versions
  - Add <concepts> header for C++20 concepts support

### Added
- **Detailed Observability Metrics** (#230): Add comprehensive metrics infrastructure for container operations
  - Add `operation_counts` struct with atomic counters for reads, writes, serializations, deserializations, copies, moves
  - Add `timing_metrics` struct with nanosecond precision for serialize/deserialize/read/write operations
  - Add `latency_histogram` struct with reservoir sampling (1024 samples) for P50/P95/P99/P999 percentiles
  - Add `simd_metrics` struct for SIMD operations, scalar fallbacks, and bytes processed tracking
  - Add `cache_metrics` struct for key index and value cache hit/miss statistics
  - Add `detailed_metrics` struct aggregating all metric types
  - Add `scoped_timer` RAII helper for automatic latency measurement
  - Add `metrics_manager` singleton for global metrics access with thread-safe atomic operations
  - Add `get_detailed_metrics()`, `reset_metrics()`, `set_metrics_enabled()`, `is_metrics_enabled()` static methods
  - Add `metrics_to_json()` for JSON export format (Grafana/ElasticSearch compatible)
  - Add `metrics_to_prometheus()` for Prometheus exposition format
  - Add enable/disable toggle for zero-overhead when metrics disabled
  - Add 9 comprehensive unit tests for metrics functionality
- **Runtime Schema Validation** (#228): Add container_schema class for data integrity validation
  - Add fluent API for schema definition with method chaining
  - Add `require(key, type)` and `optional(key, type)` for field definitions
  - Add `range(key, min, max)` for numeric constraints (int64_t and double)
  - Add `length(key, min, max)` for string/bytes length validation
  - Add `pattern(key, regex)` for regex-based string validation
  - Add `one_of(key, values)` for enum-style allowed value validation
  - Add `custom(key, validator)` for user-defined lambda validators
  - Add nested schema validation for container_value fields
  - Add `validate(container)` returning first error or nullopt
  - Add `validate_all(container)` returning all validation errors
  - Add `validate_result(container)` with Result<T> pattern support
  - Add `validation_error` struct with detailed error information
  - Add `validation_codes` namespace with specific error codes (310-317)
  - Add comprehensive unit tests (25 test cases)
  - Performance target: <1μs for simple schemas (10 fields without regex)
- **Batch Operation APIs** (#229): Add optimized batch APIs for bulk insert, query, and modify operations
  - Add `bulk_insert(vector&&)` for move-based bulk insert (most efficient)
  - Add `bulk_insert(span, reserve_hint)` for copy-based bulk insert with pre-allocation
  - Add `get_batch(keys)` returning vector of optional values (single lock acquisition)
  - Add `get_batch_map(keys)` returning map of found key-value pairs
  - Add `contains_batch(keys)` for checking multiple keys at once
  - Add `remove_batch(keys)` for removing multiple keys (returns count)
  - Add `update_if(key, expected, new_value)` for conditional compare-and-swap style update
  - Add `update_batch_if(updates)` for bulk conditional updates
  - Add `update_spec` struct for batch update specifications
  - Add Result-returning variants: `bulk_insert_result()`, `get_batch_result()`, `remove_batch_result()`
  - All batch operations use single lock acquisition for efficiency
  - Expected 3-5x performance improvement over iterative operations
  - Add comprehensive unit tests for all batch operations
- **AVX-512 SIMD Support** (#227): Add AVX-512 support for modern Intel/AMD processors
  - Add AVX-512F detection macros and 512-bit SIMD types (`float_simd_512`, `double_simd_512`, `int32_simd_512`)
  - Implement AVX-512 optimized operations (`sum_floats_avx512`, `min_float_avx512`, `max_float_avx512`)
  - Add `sum_doubles_avx512` for double precision SIMD operations
  - Add runtime detection for AVX-512 subfeatures (`has_avx512f()`, `has_avx512dq()`, `has_avx512bw()`, `has_avx512vl()`)
  - Add `simd_level` enum for SIMD capability representation
  - Add `get_best_simd_level()` to query runtime SIMD support
  - Update dispatch logic to prefer AVX-512 when available (AVX-512 > AVX2 > SSE > NEON)
  - Enhance `get_simd_info()` to show both compile-time and runtime SIMD support
  - Add comprehensive SIMD test suite (`tests/simd_tests.cpp`)
  - Expected 2x performance improvement on AVX-512 capable hardware
- **Extend Result<T> Pattern to All Public APIs** (#231): Add comprehensive Result-based error handling
  - Add standardized error codes in `error_codes` namespace (core/container/error_codes.h)
    - Value operations (1xx): key_not_found, type_mismatch, empty_key, etc.
    - Serialization (2xx): serialization_failed, deserialization_failed, etc.
    - Validation (3xx): schema_validation_failed, constraint_violated, etc.
    - Resource (4xx): memory_allocation_failed, file_not_found, etc.
    - Thread safety (5xx): lock_acquisition_failed, concurrent_modification
  - Add Result-returning value APIs:
    - `get_result(key)`: Returns `Result<optimized_value>`
    - `set_result(key, value)`: Returns `VoidResult`
    - `set_all_result(vals)`: Returns `VoidResult`
    - `remove_result(key)`: Returns `VoidResult`
  - Add Result-returning serialization APIs:
    - `serialize_result()`: Returns `Result<string>`
    - `serialize_array_result()`: Returns `Result<vector<uint8_t>>`
    - `to_json_result()`: Returns `Result<string>`
    - `to_xml_result()`: Returns `Result<string>`
  - Add Result-returning file operation APIs:
    - `load_packet_result(path)`: Returns `VoidResult`
    - `save_packet_result(path)`: Returns `VoidResult`
  - Add migration guide: docs/guides/RESULT_PATTERN_MIGRATION.md
  - All new methods marked with `[[nodiscard]]` attribute
- **Zero-Copy Deserialization** (#226): Complete zero-copy deserialization implementation
  - Add `value_view` class for non-owning access to serialized data
  - Implement lazy parsing index for on-demand value lookup
  - Add `get_view()` method for zero-copy value access
  - Add `is_zero_copy_mode()` and `ensure_index_built()` methods
  - String values can be accessed via `string_view` without copying
  - Provides 10-30x performance improvement for large message parsing
- **Memory Pool Integration** (#225): Integrate memory pool into value_container for small allocations
  - Add `internal/pool_allocator.h` with thread-local pool instances
  - Support two size classes: small (<=64 bytes) and medium (<=256 bytes)
  - Add `pool_allocate<T>()` and `pool_deallocate<T>()` template functions
  - Add `CONTAINER_USE_MEMORY_POOL` CMake option (default: ON)
  - Extend `pool_stats` structure with detailed allocation statistics
  - Add `hit_rate()` method to calculate pool efficiency
  - Expected 10-20x performance improvement for small allocations
- **C++20 Module Support** (#222): Add C++20 module files for `kcenon.container`
  - Create `src/modules/container.cppm` primary module interface
  - Export all public types: `value_types`, `value_variant`, `optimized_value`, `pool_stats`, `value_container`
  - Export `variant_helpers` namespace with JSON/XML encoding utilities
  - Import `kcenon.common` module for `Result<T>` pattern support
  - Add `CONTAINER_BUILD_MODULES` CMake option for module build
  - Requires CMake 3.28+ and Ninja generator for module compilation
  - Header-based build continues to work during transition period
- **Unified set() Method for messaging_container_builder** (#218): Add unified `set()` method to `messaging_container_builder` for API consistency
  - Add `set()` template method matching `value_container::set()` API
  - Supports method chaining for fluent interface
  - Deprecate `add_value()` with `[[deprecated]]` attribute

### Changed
- **Migrate Test Files to Unified set() API** (#218): Replace deprecated `add_value()` with `set()` in test files
  - Migrate 23 calls in `test_messaging_integration.cpp`
  - Migrate 6 calls in `performance_tests.cpp`
  - All tests pass with no regression

### Removed
- **Remove deprecation.h** (#221): Remove deprecation warning suppression macros for C++20 module preparation
  - Remove `core/deprecation.h` file with custom warning suppression macros
  - Remove `CONTAINER_SUPPRESS_DEPRECATION_START/END` macros from test framework files
  - Remove deprecated wrapper functions (`IsCiEnvironment`, `AdjustPerformanceThreshold`, `AdjustDurationThreshold`) that had no callers
  - Use standard `[[deprecated]]` attribute instead of custom macros
  - Follows Kent Beck's "Fewest Elements" principle
- **Deprecated Thread Safety Methods** (#217): Remove deprecated thread safety control methods from `value_store`
  - Remove `enable_thread_safety()`: was a no-op since v0.2.0
  - Remove `disable_thread_safety()`: was a no-op since v0.2.0
  - Remove `is_thread_safe()`: always returned true since v0.2.0
  - Thread safety is always enabled since v0.2.0 (see #190 for TOCTOU security fix)
  - **Breaking Change**: Code calling these methods will no longer compile

### Changed
- **Remove Unused Statistics Counters** (#209): Remove dead code for Simple Design compliance
  - Remove `read_count_`, `write_count_`, and `serialization_count_` atomic counters
  - Keep `heap_allocations_` and `stack_allocations_` (exposed via `memory_stats()`)
  - Reduces memory footprint by 24 bytes per container instance
  - Eliminates unnecessary atomic operations in lock guards and serialize()
- **Header Modularization** (#191): Split container.h (902 lines) into focused sub-headers
  - `core/container/fwd.h`: Forward declarations for container types
  - `core/container/types.h`: value_variant, optimized_value, pool_stats definitions
  - `core/container/variant_helpers.h`: JSON/XML encoding utilities
  - container.h now acts as umbrella header for backward compatibility
  - Improves compilation times and code navigation

### Security
- **TOCTOU Thread Safety Fix** (#190): Eliminate Time-Of-Check-Time-Of-Use vulnerability
  - Always acquire locks in `read_lock_guard` and `write_lock_guard`
  - Remove conditional locking based on runtime flag
  - Deprecate `enable_thread_safety()` and `disable_thread_safety()` methods (will be removed in v0.3.0)
  - All operations are now thread-safe by default

### Fixed
- **Clang Compatibility for Memory Pool Tests** (#225): Fix compilation error on macOS Clang
  - Add explicit constructor to SmallStruct in PoolAllocateTemplate test
  - Clang does not support C++20 parenthesized initialization of aggregates unlike GCC
  - Fix `hit_rate()` function call in memory_efficiency_bench.cpp
- **XML Entity Encoding** (#187): Add proper XML 1.0 entity encoding in `to_xml()`
  - Encode XML special characters: `&` -> `&amp;`, `<` -> `&lt;`, `>` -> `&gt;`, `"` -> `&quot;`, `'` -> `&apos;`
  - Encode control characters (0x00-0x1F except tab, newline, carriage return) as `&#xNN;`
  - Apply encoding to string values and header fields
  - Prevents invalid XML output and potential XML injection attacks
- **JSON String Escaping** (#186): Add proper RFC 8259 compliant escaping in `to_json()`
  - Escape special characters: `"`, `\`, newline, carriage return, tab, backspace, form feed
  - Escape control characters (U+0000-U+001F) as `\uXXXX` format
  - Apply escaping to both string values and field names
  - Apply escaping to header fields (source_id, target_id, message_type, version)
  - Prevents invalid JSON output and potential injection attacks

### Changed
- Remove fmt library dependency and use C++20 std::format exclusively (#168)
- Simplify formatter.h by removing conditional compilation branches
- Update CMake configuration to require C++20 std::format support

### Added
- **C++20 Concepts Integration** (#172): Apply C++20 concepts from common_system for improved type safety and clearer error messages
  - New `core/concepts.h` header with container-specific concepts
  - `ValueVariantType` concept for type-safe value storage
  - `Arithmetic`, `TriviallyCopyable` concepts for template constraints
  - `KeyValueCallback`, `MutableKeyValueCallback` concepts for iteration functions
  - Replaced `static_assert` with concept constraints for better compile-time errors
- Comprehensive documentation unification across ecosystem
- CHANGELOG.md and MIGRATION.md for better version tracking

---

## [1.0.0] - 2025-10-21

### Added
- **15 Value Types**: int, uint, long, ulong, double, bool, string, blob, array, map, and more
- **SIMD Optimization**: ARM NEON and x86 AVX support for high-performance operations
- **Multiple Serialization Formats**: Binary, JSON, XML support
- **Type-Safe Container**: Compile-time type safety with runtime flexibility
- **Zero-Copy Operations**: Efficient memory management for large data
- **Builder Pattern API**: Fluent interface for container construction
- **Memory Pool**: Optimized memory allocation for hot paths
- **Thread-Safe Operations**: Lock-free reads with mutex-protected writes
- **Integration Tests**: 49 comprehensive test cases (Phase 5)

### Performance
- Container creation: 2M containers/sec
- Serialization (binary): 1.8M containers/sec
- SIMD operations: 25M ops/sec
- Memory overhead: ~32 bytes per container

### Integration
- messaging_system: Seamless message container mapping
- network_system: Network transport integration
- database_system: Data persistence support

---

## [0.9.0] - 2025-09-15

### Added
- Initial separation from messaging_system
- Core value types implementation
- Basic serialization support

### Changed
- Modular architecture design
- Standalone build system

---

## [0.8.0] - 2025-08-01

### Added
- Binary serialization format
- Value type extensibility framework

### Fixed
- Memory leaks in array/map types
- Thread safety issues in concurrent access

---

## Version Numbering

This project uses Semantic Versioning:
- **MAJOR** version: Incompatible API changes
- **MINOR** version: Backwards-compatible functionality additions
- **PATCH** version: Backwards-compatible bug fixes

---

## Migration Guides

### Migrating to 1.0.0

See [MIGRATION.md](MIGRATION.md) for detailed migration instructions.

#### Quick Migration Example

**Before** (messaging_system 0.x):
```cpp
#include <messaging/container.h>
auto container = messaging::create_container();
```

**After** (container_system 1.0):
```cpp
#include <container/container.h>
auto container = container_system::create_container();
```

---

## Breaking Changes

### Version 1.0.0
- Namespace change: `messaging::container` → `container_system::container`
- Header paths changed to independent structure
- CMake target renamed: `messaging_container` → `kcenon::container_system`

---

## Performance Improvements

### Version 1.0.0
- SIMD optimization: +150% performance improvement
- Memory pool: -40% allocation overhead
- Lock-free reads: +200% concurrent read performance

---

## Known Issues

### Version 1.0.0
- Large types (>64 bytes) may impact cache efficiency
- JSON serialization slower than binary format (~4x)
- Windows platform requires C++20 for full SIMD support

---

## Support

- **Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- **Email**: kcenon@naver.com

---

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

---

[Unreleased]: https://github.com/kcenon/container_system/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/kcenon/container_system/releases/tag/v1.0.0
[0.9.0]: https://github.com/kcenon/container_system/releases/tag/v0.9.0
[0.8.0]: https://github.com/kcenon/container_system/releases/tag/v0.8.0
