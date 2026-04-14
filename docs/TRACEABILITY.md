---
doc_id: "CNT-QUAL-007"
doc_title: "Feature-Test-Module Traceability Matrix"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "QUAL"
---

# Traceability Matrix

> **SSOT**: This document is the single source of truth for **Container System Feature-Test-Module Traceability**.
>
> **See also**: [PRODUCTION_QUALITY.md](PRODUCTION_QUALITY.md) for the quality programme that consumes this matrix, and [contributing/CONTRIBUTING.md](contributing/CONTRIBUTING.md) for the workflow that updates it.

## Feature -> Test -> Module Mapping

### Core Container

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-001 | Value Types (16 types) | tests/unit_tests.cpp, tests/variant_value_factory_tests.cpp | include/kcenon/container/container/, src/ | Covered |
| CNT-FEAT-002 | Value Container Operations | tests/unit_tests.cpp, integration_tests/scenarios/value_operations_test.cpp | include/kcenon/container/container/ | Covered |
| CNT-FEAT-003 | Container Lifecycle | integration_tests/scenarios/container_lifecycle_test.cpp | include/kcenon/container/container/ | Covered |
| CNT-FEAT-004 | Error Codes | tests/error_codes_tests.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-005 | Schema Validation | tests/schema_tests.cpp | include/kcenon/container/container/ | Covered |
| CNT-FEAT-006 | Result\<T\> Getter API | tests/result_api_tests.cpp | include/kcenon/container/container/ | Covered |

### Serialization

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-007 | Binary Serialization | tests/unit_tests.cpp | include/kcenon/container/serializers/ | Covered |
| CNT-FEAT-008 | JSON Serialization | tests/unit_tests.cpp | include/kcenon/container/serializers/ | Covered |
| CNT-FEAT-009 | XML Serialization | tests/unit_tests.cpp | include/kcenon/container/serializers/ | Covered |
| CNT-FEAT-010 | MsgPack Serialization | tests/msgpack_tests.cpp | include/kcenon/container/serializers/ | Covered |
| CNT-FEAT-011 | Serializer Factory | tests/serializer_factory_tests.cpp | include/kcenon/container/serializers/ | Covered |
| CNT-FEAT-012 | Fast Parser Optimization | tests/fast_parser_integration_tests.cpp | include/container/optimizations/ | Covered |

### SIMD Optimization

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-013 | SIMD Processing (NEON/AVX2) | tests/simd_tests.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-014 | SIMD Policies | tests/simd_policies_tests.cpp | include/kcenon/container/ | Covered |

### Thread Safety

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-015 | Thread-Safe Container | tests/thread_safety_tests.cpp | include/kcenon/container/ | Covered |

### Memory Management

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-016 | Memory Pool | tests/memory_pool_tests.cpp, tests/memory_pool_benchmark.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-017 | Storage Policies | tests/storage_policy_tests.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-018 | Policy Container | tests/policy_container_tests.cpp | include/kcenon/container/ | Covered |

### Async API (C++20 Coroutines)

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-019 | task\<T\> / generator\<T\> / async_container | tests/async_tests.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-020 | Thread Pool Executor | tests/thread_pool_executor_tests.cpp | include/kcenon/container/ | Covered |

### Messaging Integration

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-021 | Messaging Integration | tests/test_messaging_integration.cpp | include/kcenon/container/ | Covered |

### Validation & Edge Cases

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-022 | Long Range Checking | tests/test_long_range_checking.cpp | include/kcenon/container/ | Covered |
| CNT-FEAT-023 | Variant Value Factory | tests/variant_value_factory_tests.cpp | include/kcenon/container/ | Covered |

### Performance

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-024 | Performance Benchmarks | tests/performance_tests.cpp, tests/benchmark_tests.cpp | (cross-cutting) | Covered |
| CNT-FEAT-025 | Serialization Performance | integration_tests/performance/serialization_performance_test.cpp | (cross-cutting) | Covered |

### Error Handling Integration

| Feature ID | Feature | Test File(s) | Module/Directory | Status |
|-----------|---------|-------------|-----------------|--------|
| CNT-FEAT-026 | Error Handling Integration | integration_tests/failures/error_handling_test.cpp | (cross-cutting) | Covered |

## Coverage Summary

| Category | Total Features | Covered | Partial | Uncovered |
|----------|---------------|---------|---------|-----------|
| Core Container | 6 | 6 | 0 | 0 |
| Serialization | 6 | 6 | 0 | 0 |
| SIMD Optimization | 2 | 2 | 0 | 0 |
| Thread Safety | 1 | 1 | 0 | 0 |
| Memory Management | 3 | 3 | 0 | 0 |
| Async API | 2 | 2 | 0 | 0 |
| Messaging Integration | 1 | 1 | 0 | 0 |
| Validation & Edge Cases | 2 | 2 | 0 | 0 |
| Performance | 2 | 2 | 0 | 0 |
| Error Handling | 1 | 1 | 0 | 0 |
| **Total** | **26** | **26** | **0** | **0** |

## See Also

- [FEATURES.md](FEATURES.md) -- Detailed feature documentation
- [README.md](README.md) -- SSOT Documentation Registry
